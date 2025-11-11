#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>

namespace mmu {

/// Returns `p` as a pointer to `T`.
///
/// Use this function when you need to reinterpret memory referred to some pointer of type `S` as
/// an instance of `T`. For example, if `p` points to an array of `std::byte`, you can `rebind` to
/// read an 32-bit integer at the address referred to by `p`:
///
///     std::uint32_t n = *rebind<std::uint32_t>(p);
///
/// The address referred to by `p` must satisfy the alignment requirements of `T`. Otherwise, the
/// result is unspecified and dereferencing the returned pointer has undefined behavior.
template<typename T>
inline constexpr T* rebind(void* p) { return static_cast<T*>(p); }

/// Returns `p` as a pointer to `T const`.
///
/// This function overloads `rebind` for pointers to immutable data.
template<typename T>
inline constexpr T const* rebind(void const* p) { return static_cast<T const*>(p); }




//////////////////////////////////////////// ADDRESSES ////////////////////////////////////////////
/// Writes a textual representation of `a`, which is an address, to `o`.
std::ostream& write_address(std::ostream& o, std::uint16_t a) {
  std::ios cout_state(nullptr);
  cout_state.copyfmt(std::cout);
  std::cout << "0x";
  std::cout << std::hex << std::setw(4) << std::setfill('0') << a;
  std::cout.copyfmt(cout_state);
  return o;
}
//////////////////////////////////////////// VIRTUAL ADDRESS ////////////////////////////////////////////
/// An index in a page translation table together with an offset in a physical frame.
///
/// A virtual address is represented as a 16-bit unsigned integer that encodes an index in page
/// translation table, which identifies a page, and an offset in the phyiscal frame corresponding
/// that corresponds to that frame.
///
/// The index is expressed as a triple of directory indices (see `Machine::translate`), encoded in
/// the 8 highest bit of the address as follows:
///
///     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
///     │ f │ e │ d │ c │ b │ a │ 9 │ 8 │ 7 │ 6 │ 5 │ 4 │ 3 │ 2 │ 1 │ 0 │
///     ╞═══╧═══╪═══╧═══╧═══╪═══╧═══╧═══╪═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╡
///     │ L0    │ L1        │ L2        │ offset in frame               │
///     └───────┴───────────┴───────────┴───────────────────────────────┘
///
/// An address whose representation is equal to zero is called a *null address*.
struct VirtualAddress {

  /// The raw value of this address.
  std::uint16_t raw;

  VirtualAddress(std::uint16_t raw) : raw(raw) {};

  /// Returns the address of the page containing this address.
  inline constexpr VirtualAddress page() const {
    return VirtualAddress{static_cast<std::uint16_t>(raw & ~0xff)};             // le tild veut dire non. ici, le ~0xff fait une masque sur raw. 
  }                                                                            // Donc le not0xff fait une masque de 1111111100000000 (ou l'inverse ?)

  /// Returns this address advanced by the given offset.
  inline constexpr VirtualAddress advanced(std::uint16_t offset) const {
    return VirtualAddress{static_cast<std::uint16_t>(raw + offset)};
  }

};

/// Writes a textual representation of `va` to `o`.
inline std::ostream& operator<<(std::ostream& o, VirtualAddress va) {   // pretty print of virtual address.
  return write_address(o, va.raw);
}

//////////////////////////////////////////// PHYSICAL ADDRESS ////////////////////////////////////////////
/// An offset in the physical address space.
struct PhysicalAddress {

  /// The raw value of this address.
  std::uint16_t raw;

};

/// Writes a textual representation of `pa` to `o`.
inline std::ostream& operator<<(std::ostream& o, PhysicalAddress pa) {   // pretty print of physical address.
  return write_address(o, pa.raw);
}
//////////////////////////////////////////// ADDRESSES END ////////////////////////////////////////////





//////////////////////////////////////////// ERROR HANDLING ////////////////////////////////////////////
/// The reason that caused a page lookup error.
enum PageLookupErrorCause : std::uint8_t {

  /// A page table entry has been resolved but the corresponding frame is not in main memory.
  PageFault = 1,

  /// The page is not mapped.
  SegmentationFault = 2,

  /// The protection of the page does not allow the requested action.
  PermissionFault = 4

};

/// An error indicating that page lookup failed.
struct PageLookupError final : public std::exception {

  /// The address that was looked up.
  const VirtualAddress target;

  /// The reason for this error.
  const PageLookupErrorCause cause;

  /// Creates an instance describing that `cause` occurred while looking up `target`.
  constexpr PageLookupError(
    VirtualAddress target, PageLookupErrorCause cause
  ) : target(target), cause(cause) {}

  /// Returns a textual description of this error as a null-terminated string.
  constexpr const char* what() const noexcept override {                        // pretty print text of the exception
    switch (cause) {
      case PageFault:
        return "page fault";
      case SegmentationFault:
        return "segmentation fault";
      case PermissionFault:
        return "permission fault";
    }
  }

};
//////////////////////////////////////////// END ERROR HANDLING ////////////////////////////////////////////



//////////////////////////////////////////// FRAME ////////////////////////////////////////////
/// Information about a frame, i.e., the storage of a page in physical memory.
///
/// A frame descriptor is a record describing a region of physical memory that forms a frame (i.e.,
/// the storage of a virtual page). The descriptor of a frame `f` is logically described as a
/// 4-tuple `(r, p, br, pp)` where:
///
/// - `r` is a flag set iff `f` has been referenced since the last stealing pass (see below),
/// - `p` is a flag set iff `f` cannot be evicted (see below),
/// - `br` is a sequence of "back" references to PTEs referring to `f`, and
/// - `pp` optionally identifies the permanent position of `f` in secondary memory.
///
/// These components are represented as an array of four 8-bit unsigned integers:
///
///     ┌────────╥───┬───┬───┬───┬───┬───┬───┬───┐
///     │        ║ 7 │ 6 │ 5 │ 4 │ 3 │ 2 │ 1 │ 0 │
///     ╞════════╬═══╧═══╪═══╪═══╪═══╧═══╪═══╪═══╡
///     │ raw[0] ║ pp hi │ - │ - │ brcnt │ p │ r │
///     ├────────╫───────┴───┴───┴───────┴───┴───┤
///     │ raw[1] ║ pp lo                         │
///     ├────────╫───────────────────────────────┤
///     │ raw[2] ║ backref 0                     │
///     ├────────╫───────────────────────────────┤
///     │ raw[3] ║ backref 1                     │
///     └────────╨───────────────────────────────┘
///
/// Note that bits 4 and 5 are currently reserved.
///
/// The permenent position identifier (`pp`) is represented as a 10-bit unsigned integer. Its two
/// highest bits are stored in `raw[0]` whereas the other bits are stored in `raw[1]`.
///
/// The sequence of back references `br` is used to update the page translation table when a frame
/// is swapped out. This feature is only partially implemented. Currently, if there are less than
/// three references, then `brcnt` contains the length of the sequence, which is stored in `raw[2]`
/// and `raw[3]` as an array of offsets in physical memory. Otherwise, `brcnt` is equal to 3 and
/// the contents of `raw[2]` and `raw[3]` is unspecified.
///
/// When a page not currently in *main memory* is accessed (or allocated), the system must first map
/// that page to some frame. If all frames are occupied, the system will try to "steal" the frame
/// from one of the current pages. The victim is selected by looking at the `r` flag of each frame.
/// If it is unset (and the frame is not pinned), then the corresponding page will be evicted.
/// Otherwise, the `r` flag is reset and the system will look for a better candidate. The `r` flag
/// of the frame will be set again after the next access to the corresponding page. This strategy
/// favors the eviction of pages that are not under heavy use.
struct FrameDescriptor {

  /// The raw representation of this frame descriptor.
  std::uint8_t raw[4];

  /// Resets the valud of this descriptor.
  inline void reset() {
    std::fill_n(raw, 4, 0);
  }

  /// Returns `true` iff the frame has been referenced since the last page stealing pass.
  inline constexpr bool is_referenced() const {
    return raw[0] & 1;
  }

  /// Sets the referenced flag of this entry.
  inline void set_referenced(bool v) {
    raw[0] = v ? (raw[0] | 1) : (raw[0] & ~1);
  }

  /// Returns `true` iff the frame can never be evicted.
  inline constexpr bool is_pinned() const {
    return raw[0] & 2;
  }

  /// Sets the pinned flag of this entry.
  inline void set_pinned(bool v) {
    raw[0] = v ? (raw[0] | 2) : (raw[0] & ~2);
  }

  /// Returns `true` iff the frame is not pinned and has not been referenced.
  ///
  /// The result of this method is equivalent to `!is_referenced() && !is_pinned()`.      // SO if a frame was recently accessed, then it's 'r' flag is at 1.
  inline constexpr bool is_ready_for_eviction() const {                                  // Basically works as a cache.
    return ((raw[0] & 3) ^ 1) == 1;
  }

  /// Returns the number of entries in the back-reference list of this frame.
  inline constexpr std::uint8_t back_reference_count() const {
    return raw[0] >> 2;
  }

  /// A list of length greater than 2 is considered to have an arbitrary length.
  /// We store the offset of the page entry.
  void add_back_reference(std::uint8_t entry) {
    auto c = back_reference_count();
    if (c < 2) {
      raw[c + 2] = entry;
    }
    if (c < 3) {
      raw[0] = (raw[0] & 0xc3) | ((c + 1) << 2);
    }
  }

  /// Removes all back references from this frame.
  void clear_back_references() {
    raw[0] = raw[0] & 0xc3;
  }

  /// Returns the offsets of the pate table entries mapping to this frame.
  inline constexpr std::uint8_t const* back_references() const {
    return raw + 2;
  }

  /// Returns the position of the frame in secondary memory if it is permanent or 0 otherwise.
  inline constexpr std::uint16_t permanent_position() const {
    return static_cast<std::uint16_t>(raw[1]) | (static_cast<std::uint16_t>(raw[0] & 0xc0) << 8);
  }

  /// Returns the position of the frame in secondary memory if it is permanent or 0 otherwise.     // I presume this is a setter and the one above is a getter.
  inline void set_permanent_position(std::uint16_t p) {
    raw[1] = static_cast<std::uint8_t>(p & 0xff);
    raw[0] = (raw[0] & 0x3f) | static_cast<std::uint8_t>(p >> 8);
  }

};
//////////////////////////////////////////// END FRAME ////////////////////////////////////////////


//////////////////////////////////////////// PAGE ////////////////////////////////////////////
/// Information about a page.
///
/// A page table entry (PTE) is a record describing how a page in the virtual memory space is     // Just as a reminder, pages go inside a frame.
/// mapped to a frame in physical memory space.
///
/// A PTE is represented as a 16-bit integer, using the layout below:
///
///     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
///     │ f │ e │ d │ c │ b │ a │ 9 │ 8 │ 7 │ 6 │ 5 │ 4 │ 3 │ 2 │ 1 │ 0 │
///     ╞═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╪═══╪═══╪═══╪═══╪═══╪═══╡
///     │ frame number                          │ - │ x │ w │ r │ p │ a │
///     └───────────────────────────────────────┴───┴───┴───┴───┴───┴───┘
///
/// The meaning of the flags stored in the least significant bits is as follows:
///
/// - `a` is set iff the page entry is defined.                                 // Does the frame already exist ?
/// - `p` is set iff the corresponding frame is present in main memory.        // present in main memory ? if not must be swapped into main memory b4 accessing it. 
/// - `r` is set iff the page can be read.                                    // Perms...
/// - `w` is set iff the page can be written to.
/// - `x` is set iff the page can be executed.
///
/// Note that bit 5 is currently reserved.
///
/// If `a` is not set, the other bits must be set to 0 and the whole representation denotes the
/// absence of any value. This invariant can be used to represent an optional PTE without using
/// additional storage.
///
/// If `p` is set, the frame number identifies a frame in main memory. Otherwise, it identifies a
/// frame in secondary memory that should be swapped in before it can be accessed.
struct PageEntry {

  /// The raw representation of a set of protection flags.
  using Protection = std::uint8_t;

  /// A bit indicating that a page can be read.
  static constexpr Protection read = 1;

  /// A bit indicating that a page can be written to.
  static constexpr Protection write = 2;

  /// A bit indicating that a page can be executed.
  static constexpr Protection execute = 4;

  /// The raw representation of the entry.
  std::uint16_t raw;

  /// Creates a "none" value.
  inline constexpr PageEntry() : raw(0) {}

  /// Creates an entry referring to the given frame.
  inline constexpr PageEntry(std::uint8_t frame) : raw(0) {
    set_frame(frame);
  }

  /// Returns `true` iff this entry does not identify any page.
  inline constexpr bool is_none() const {
    return raw == 0;
  }

  /// Returns `true` iff the frame corresponding to this entry is present in main memory.
  inline constexpr bool is_present() const {
    return raw & 2;
  }

  /// Sets the present flag of this entry.
  inline void set_present(bool v) {
    raw = v ? (raw | 3) : (raw & ~2);
  }

  /// Returns the protection flags of this frame.
  inline constexpr Protection protection() const {
    return (raw >> 2) & 3;
  }

  /// Modifies the protection flags of this frame.
  inline void set_protection(Protection f) {
    raw = 1 | (raw & ~0x1c) | (f << 2);
  }

  /// Returns the offset of the frame corresponding to the page described by this entry.
  inline constexpr std::uint16_t frame() const {
    return static_cast<std::uint8_t>(raw >> 6);
  }

  /// Modifies the offset of the frame corresponding to the page described by this entry.
  inline constexpr void set_frame(std::uint16_t value) {
    raw = 1 | (raw & 0x3f) | (value << 6);
  }

  /// Creates an instance from its raw value.
  static constexpr PageEntry from_raw(std::uint16_t raw) {
    PageEntry self;
    self.raw = raw;
    return self;
  }

};
//////////////////////////////////////////// END FRAME ////////////////////////////////////////////




//////////////////////////////////////////// TLB ////////////////////////////////////////////
/// A translation lookaside buffer (TLB).
///
/// This data structure implements a cache backed by a simple ring buffer. Each entry in the cache
/// is represented by a pair of 16-bit integers whose components are a 16-bit aligned address and
/// the raw value of a page table entry (PTE), respectively.
///
/// An empty entry is represented by a pair of zeros, relying on the fact that the null address has
/// no translation and that the raw value of an empty PTE is zero.
template<std::size_t size>
struct TLB {

  /// The elements in this TLB.
  std::uint32_t elements[size] = {0};

  /// The index of the first element in this TLB.
  std::uint32_t first = 0;

  /// Accesses the `i-th` element of this TLB.
  inline constexpr std::uint32_t operator[](std::size_t i) const {
    return elements[(first + i) & (size - 1)];
  }

  /// Accesses the `i-th` element of this TLB.
  inline constexpr std::uint32_t& operator[](std::size_t i) {
    return elements[(first + i) % size];
  }

  /// Inserts a record in this TLB, mapping the page-aligned address `va` to the page entry `pte`.
  ///
  /// - Requires: there is no record mapping `va` to a page entry before the method is called.
  void insert(VirtualAddress va, PageEntry pte) {
    first = (first - 1) % size;
    elements[first] = (pte.raw << 16) | va.raw;
  }

  /// Returns the cached page table entry corresponding to the given page-aligned address, if any.
  ///
  /// The method returns an empty PTE (i.e., a PTE whose raw value is zero) iff the cache contains
  /// no entry mapping `va`. Otherwise, the looked-up entry is moved closer to the start.
  PageEntry lookup(VirtualAddress va) {                                                           // This is the circular array thingy. Called a ring array.
    for (std::size_t i = 0; i < size; ++i) {
      auto const p = (*this)[i];
      auto const k = p & 0xffff;

      // Did we reach the end of the buffer?
      if (k == 0) { break; }

      // Is that a hit?
      else if (va.raw == k) {
        // Update the cache and return the decoded entity.
        if (i > 0) { std::swap((*this)[i], (*this)[i - 1]); }
        return PageEntry::from_raw(static_cast<std::uint16_t>(p >> 16));
      }
    }

    // Out of luck.
    return PageEntry{};
  }

  /// Invalidates all enries referring to `pte`.
  void invalidate(PageEntry pte) {
    // Use `i` to point at the first of the entries left to process and `j` to point immediately
    // after the last entry to process.
    std::size_t i = 0, j = size;

    // The loop maintain the invariant that all entries after `j` are 0. Termination is guaranteed
    // by the fact that each iteration either increases `i` or decreases `j`.
    while (i < j) {
      if (((*this)[i] >> 16) == pte.raw) {
        (*this)[i] = 0;
        std::swap((*this)[i], (*this)[j - 1]);
        j--;
      } else {
        i++;
      }
    }
  }

};

            // Above is a pretty important algorithm (swap) because it has a linear complexity of O(n).

//////////////////////////////////////////// END TLB ////////////////////////////////////////////










/// A virtual machine and its operating system.
///
/// Instances of this type model a virtual machine equipped with a single-core CPU, 4KB of main
/// memory, and 256KB of secondary memory.
///
/// The system implements demand *paging*. The virtual address space is represented using the full
/// range of a 16-bit unsigned integer, mapping onto 12-bit physical address space, segmenting
/// memory into 256 bytes pages. Address mappings are encoded by a multi-level page translation
/// table indexed by the 8 most significant bits of a virtual address.
struct Machine {

  /// The number of entries in the TLB.
  static constexpr std::size_t tlb_size = 4;

  /// The description of a contiguous region in memory.
  struct RegionPointer {

    /// The raw representation of this pointer.
    std::uint16_t raw[4];

    /// The offset of the region.
    inline std::uint16_t& offset() { return raw[0]; }

    /// The length of the region.
    inline std::uint16_t& length() { return raw[1]; }

  };

  /// The translation lookaside buffer of the machine.
  TLB<tlb_size> tlb;

  /// The main memory of the machine.
  ///
  /// The system has 4KB of memory, divided in 16 pages of 256 bytes.
  alignas(alignof(std::uint32_t)) std::byte main_memory[4096] = {};

  /// The secondary memory of the machine.
  ///
  /// Secondary memory is also divided in frame slots of 256 bytes. The first slot is reserved to
  /// store a pointer to the next available region. A region `r` is denoted by a pair `(o, l)`
  /// where `o` is the offset to the start of the region and `l` is its length.
  std::byte* secondary_memory;

  /// A table describing how each frame of the main memory is occupied.
  ///
  /// This table occupies 64 byte: 16 x 4 bytes for each frame that can reside in main memory.
  inline FrameDescriptor* frame_table() {
    return rebind<FrameDescriptor>(main_memory);
  }

  /// A table indicating which frames are used.
  ///
  /// This table occupies 2 bytes: 16 bits to indicate which main memory slot is free.
  inline std::uint16_t& free_map() {
    return *rebind<std::uint16_t>(main_memory + 64);
  }

  /// The position of the "kernel break".
  ///
  /// The kernel break is the first 16-byte aligned physical address that the kernel may use to
  /// allocate heap memory.
  ///
  /// This property occupies 2 bytes: 16 bits to store the offset of the program break.
  inline std::uint16_t& kbrk() {
    return *rebind<std::uint16_t>(main_memory + 64 + 2);
  }

  /// The root of the page translation table (aka pbtr).
  ///
  /// This table occupies 8 bytes: 4 x 2 bytes for each entry in the base page table.
  inline std::uint16_t* page_map() {
    return rebind<std::uint16_t>(main_memory + 64 + 2 + 2);
  }

  /// The number of bits by which an address should be shifted to the right to read the index of
  /// `(i + 1)`-th translation directory level.
  static constexpr std::uint16_t shifts[3] = {11, 8};

  /// The mask to apply to read the index of the `(i + 1)`-th translation directory level.
  static constexpr std::uint16_t masks[2] = {0xc0ff, 0xf8ff};

  /// Creates an instance and initializes its page translation table.
  ///
  /// The main data structures of the kernel (e.g., the page map) are stored in the first frame,
  /// which is pinned and therefore never evicted. Hence, offsets from 0 in the physical address
  /// space can be used as stable addresses to these data structures and their contents.
  ///
  /// The translation table is initialized so that addresses in the range from `0xf800` to `0xf8ff`             // Prolly important addresses
  /// map to the first frame of the main memory, which is pinned.
  Machine() {
    // Allocate the secondary memory on the heap.
    secondary_memory = new std::byte[262144];
    *rebind<RegionPointer>(secondary_memory) = {1, 1023};

    // Zero-out the main memory.
    std::fill_n(main_memory, 4096, std::byte{0});

    // Initialize the translation table so that addresses from 0xf800 to 0xf8ff map to the first
    // frame of the main memory, which is pinned. This frame is used to store the kernel's main
    // data structures, such as the page map and the frame table.
    auto* frame_table = this->frame_table();
    auto* page_map = this->page_map();

    // Compute a 16-aligned offset after the page map to form the kernel break.
    auto brk = std::distance(main_memory, rebind<std::byte>(page_map + 4));
    brk += -(brk & 15) & 15;

    // Allocate d0 right after the frame table and the page map.
    auto* d0 = rebind<std::uint16_t>(main_memory + brk);
    this->kbrk() = brk + 16;

    // Assign the (physical) address of d0 to page_map[3].
    page_map[3] = brk;

    // Write a single page table entry at `d0[7]`.
    PageEntry pte;
    pte.set_present(true);
    pte.set_protection(PageEntry::read | PageEntry::write);
    pte.set_frame(0);
    d0[7] = pte.raw;

    frame_table[0].set_pinned(true);
    frame_table[0].add_back_reference(brk);
    this->free_map() = 0xfffe;
  }

  ~Machine() {
    delete[] secondary_memory;
  }

  /// Reads a byte from physical address `pa`.
  inline constexpr std::byte read_byte(PhysicalAddress pa) const {
    return this->main_memory[pa.raw];
  }

  /// Stores `b` at physical address `pa`.
  inline void store_byte(std::byte b, PhysicalAddress pa) {
    this->main_memory[pa.raw] = b;
  }

  /// Allocates `byte_count` bytes of memory from the kernel's heap.
  std::uint16_t kalloc(std::size_t byte_count) {
    auto a = this->kbrk();
    auto b = a + byte_count;
    auto c = b + (-(b & 15) & 15);

    // Did we run out of memory (max kernel occupies the two first frames).
    if (c < 0x0200) {
      this->kbrk() = c;
      return a;
    } else {
      throw std::bad_alloc();
    }
  }

  /// Returns the offset of the given page entry.
  ///
  /// `pte` is a pointer to a page entry that is stored in the machine's main memory.
  inline std::uint8_t pte_offset(PageEntry const* pte) const {
    return static_cast<std::uint8_t>(std::distance(this->main_memory, rebind<std::byte>(pte)));
  }

  /// Returns the physical address corresponding to `va` accessed with `permissions`, knowing that
  /// `va` resides in the page described by `pte`.
  ///
  /// This method is called internally after the page entry corresponding to `va` has been found,
  /// as a result of either TLB hit or a page walk. It throws if the protection of the page do not
  /// support `permissions`.
  ///
  /// If the frame storing the page is not in main memory, its contents is retrived from secondary
  /// memory and `pte` is updated to refer to physical location (in main memory) of the frame where
  /// the page's contents has been written.
  ///
  /// If the method does not throw and `update_tlb` is true, the TLB is updated once the frame
  /// containing the translated address has been properly configured.
  PhysicalAddress translate_with_entry(
    VirtualAddress va, PageEntry::Protection permissions, PageEntry& pte, bool update_tlb
  ) {
    // Does the page have the right protection?
    if ((pte.protection() & permissions) != permissions) {
      throw PageLookupError{va, PermissionFault};
    }

    std::uint16_t frame_index = 0xff;

    // Is the page in main memory?
    if (pte.is_present()) {
      frame_index = pte.frame();
      if (update_tlb) { tlb.insert(va.page(), pte); }
    }

    // Otherwise, the frame number contains the location where the page has been swapped out.
    else {
      frame_index = swap_victim(this, pte.frame());
      pte.set_frame(frame_index);
      pte.set_present(true);
      if (update_tlb) { tlb.insert(va.page(), pte); }

      // Update the frame so that it points back to the page table entry that has been modified.
      // We can assume `pte` refers directly to some location in the page translation table since
      // the referred frame would have been present in memory otherwise.
      this->frame_table()[frame_index].add_back_reference(pte_offset(&pte));
    }

    this->frame_table()[frame_index].set_referenced(true);
    return PhysicalAddress{static_cast<uint16_t>((frame_index << 8) | (va.raw & 0xff))};
  }

  /// Returns the physical address corresponding to `va` accessed with `permissions`, handling
  /// segfaults with `handle_segfault`.
  ///
  /// The translation first checks whether the page containing `va` is in the TLB. If it is, the
  /// result can be computed immediately. Otherwise, a TLB miss occurs. Note that this step is
  /// typically carried out by the hardware in a real system.
  ///
  /// If a TLB miss occurs, the system decodes the 8 most significant bits of `va` to index the
  /// page translation table (see `VirtualAddress`). This process is referred to as a page walk.
  ///
  /// The page translation table has three levels, denoted by *L0*, *L1*, and *L3*, respectively.   // I presume that *L3* is L2 instead
  /// Each level is composed of a number of directories that represent nodes or leaves in a tree.
  /// Specifically, *L0* encodes the root node and consists of only one directory with 4 entries,
  /// *L1* can contain up to 4 directories (one per child of the root) of 8 entries, and *L2* can
  /// contain up to 32 directories (one per child at *L1*) of 8 entries.
  ///
  /// Each directory entry is represented as a 16-bit unsigned integer, which encode a union of
  /// three different type of values.
  /// - If the entry is equal to zero, then any translation going through the corresponding node
  ///   should fail with a segfault.
  /// - If the least significant bit of the entry is set, then it encodes a PTE directly and the
  ///   page walk can end.
  /// - Otherwise, the entry encodes the address of a directory in the next level as an offset in
  ///   physical memory.
  ///
  /// Note that the directory entries at *L2* can only be equal to zero or encode a PTE since the
  /// translation table has only three levels.
  ///
  /// The method calls `handle_segfault(this, va, permissions, i)` if `va` is non-null address that
  /// is not currently mapped, where `i` is the level at which the fault was detected. The handler
  /// is expected to either return the value that would have been found at level `i` if the page
  /// was mapped or throw an exception.
  ///
  /// The method throws if the protection of the page do not support `permissions`.
  template<typename F>
  PhysicalAddress translate(
    VirtualAddress va, PageEntry::Protection permissions, F&& handle_segfault
  ) {
    // The null address has no translation.
    if (va.raw == 0) { throw PageLookupError(va, SegmentationFault); }

    // Check the TLB.
    auto pte = tlb.lookup(va.page());
    if (!pte.is_none()) {
      assert(pte.is_present());
      return translate_with_entry(va, permissions, pte, false);
    }

    // Walk the page table.
    auto* pda = page_map() + (va.raw >> 14);

    for (auto i = 0; i < 2; ++i) {
      // The null address has no translation.
      if (*pda == 0) {
        handle_segfault(this, va, permissions, pda, i);
        assert(*pda != 0);
      }

      // If the least significant bit of `pda` is set, then it encodes a page entry rather than a
      // directory address.
      if (*pda & 1) {
        // Make sure the remaining directory bits are zeroed-out.
        if ((va.raw & ~masks[i]) != 0) { throw PageLookupError(va, SegmentationFault); }

        // Decode the page entry.
        return translate_with_entry(va, permissions, *rebind<PageEntry>(pda), true);
      }

      // If `pda` is less than 4096 it denotes a physical address in main memory.
      else if (*pda < 4096) {
        auto* directory = rebind<std::uint16_t>(main_memory + *pda);
        pda = directory + ((va.raw >> shifts[i]) & 0x7);
      }

      // Otherwise, we're out of luck.
      else { std::abort(); }
    }

    if (*pda == 0) {
      handle_segfault(this, va, permissions, pda, 2);
      assert(*pda != 0);
    }

    // If we got there, `pda` encodes the raw contents of some page table entry.
    return translate_with_entry(va, permissions, *rebind<PageEntry>(pda), true);
  }









  /// Returns the physical address corresponding to `va` accessed with `permissions`, throwing
  /// if `va` is not mapped or if the protection of the page are incompatible with `permissions`.
  PhysicalAddress translate(VirtualAddress va, PageEntry::Protection permissions) {
    return translate(va, permissions, &Machine::rethrow);
  }

  /// Allocates a page at the page-aligned address `va`, assuming it isn't already allocated.
  PhysicalAddress allocate_page(VirtualAddress va, PageEntry::Protection ps) {
    return translate(va, ps, &Machine::allocate_on_segfault);
  }

  /// Creates a new mapping in the virtual address space with the specified `length` and
  /// `protection`.
  ///
  /// If `hint` is null, then the kernel chooses the (page-aligned) address at which to create the
  /// mapping. If `hint` is not null, then the kernel picks a nearby page boundary and attempts to
  /// create the mapping there. If another mapping already exists there, the kernel picks a new
  /// address that may or may not depend on the hint.
  ///
  /// The address of the new mapping is returned as the result of the call.
  VirtualAddress simple_mmap(
    VirtualAddress hint, std::size_t length, PageEntry::Protection protection) {
         // So I presume I'm supposed to find a new location to fit data of size size_t with specific perms.
        // When using this function, we are given this:
       //            m.simple_mmap(0, 128, PageEntry::read | PageEntrey::Write)
      // This is found inside of the test-all.cc file. (I was confused what we were given when this function
     // is called). I think you can access the bit values by using x.raw ?
    // Pretty sure the length is bytes.. I'm assuming, so if something's wrong.. then it's probably this.

     // This is the final result, which is a 16bit int as described at line 48 and is the value we return.
    VirtualAddress resultVirtualAddress = 0;

     // The 8LSB together are the frame offset.
    // The 2MSB is L0, next 3 L1 and final 3 L3.


    //***********OVERVIEW *************//
       // - We are given a wanted size with perms.
      // - Perms are handled in the page section. (more precisly the Page Table Entry)
     // - Pages are contained withing frames..
    // - Hunh..

        // So basically, we have to start with a hint which hints us where to start.
       // Then we have a size.
      // From that size, we have to access the the physical memory.
     // Find and create a frame.
    // From that frame insert a page in which we assign the correct permissions.

    // Starting from scratch. We want:
    // To export a virtual address that is one 16bit integer. It contains the path to the right area ?
    // We reach the corresponding frame, from which we can reach the correct page.
    // Something with the translation lookaside buffer.

    // A lot of throaway comments.



       // Firstly, when it means the hint is null.. I believe it means that it's a 
      // null address, therefor it's representation is = 0. So the first if statement is if (hint == 0).
     // If true, then we start at 0x1000.
    // And if not, then we just start at hint.
    if (hint.raw == 0) {
      resultVirtualAddress = VirtualAddress{0x1000};
    } else {
      resultVirtualAddress = hint;
    }

         // Next the size.. so need to know how many pages we need to fit the entire data into.
        // One page is 256bytes so we would need length / 256 but rounded up cause if 
       // llength = 120 => nbrPages = 0.xxx. Just taking the same type as size: size_t.
      // std::size_t nbrPages = ceil(length / 256)
     // damn can't use ceil without importing a library..
    // Just do it manually lmao just by adding 256 before dividing
    std::size_t nbrPages = (length + 256) / 256;


    // Have to align it
    resultVirtualAddress = resultVirtualAddress.page();


         // Now for the searching: need to keep going until found, or out of bounds.
        // So will have the bounds as the condidtion and a break function as found at the end.
       // hehe commented the kernel bounds code when reading it.. preshoted
      // So the start of the kernel space is definitly 0xf800 and goes to 0xffff I imagine,
     // since the comments said that 0xf800 to 0xf8ff was the translation table, which
    // is in the kernel space and goes up => kernel space upper bound 0xffff.
    std::size_t kernelLowerBound = 0xf800;
    bool foundOne = false;
    VirtualAddress startingPoint = resultVirtualAddress;

    // Looks for a section containing enough pages to satisfy the date => nbrPages.
    while (resultVirtualAddress.raw < kernelLowerBound){
      // added try/catch after to resolve the gad dam error handling.
      try{
        foundOne = true;
        for(std::size_t i = 0; i < nbrPages && foundOne; i++){    // goes through all pages one by one
          translate(resultVirtualAddress.advanced(i * 256), 0);  // with the premade translate function
          foundOne = false;                                     // we can see if it has the right perms
        }                                                      // and throws to catch if perms don't 
        } catch (PageLookupError const& e) {                  // allow or if va not mapped => means it's freeee.
            if (e.cause == SegmentationFault) {              // SegementationFault means Page was not mapped
              if (foundOne) break;                          // but if it found an area that is free, it just
            }                                              // exits.
      }

      if (!foundOne) {                                  // If didn't find one, just try the next page
        resultVirtualAddress = resultVirtualAddress.advanced(256);
         // explained by a friend :
        // If it goes back to the hint address, then there is no space available
        if (resultVirtualAddress.raw == startingPoint.raw) {
          throw std::bad_alloc();
        }
      }
    }

    // Don't even need this part.
    //if (!foundOne || resultVirtualAddress.raw >= 0xf800) { throw std::bad_alloc(); } // Truely the worst

    // Finally allocate all the pages legally.
    for (std::size_t i = 0; i < nbrPages; ++i) {
      allocate_page(resultVirtualAddress.advanced(i * 256), protection);
    }

    return resultVirtualAddress;
  };
  // Yippee the end.





  /// A segfault handlers that simply throws.
  static void rethrow(
    Machine*, VirtualAddress va, PageEntry::Protection, std::uint16_t*, std::size_t
  ) {
    throw PageLookupError(va, SegmentationFault);
  }

  /// A segfault handlers that allocates a frame for `va`.
  static void allocate_on_segfault(
    Machine* self, VirtualAddress va, PageEntry::Protection ps, std::uint16_t* pda, std::size_t i
  ) {
    assert(*pda == 0);

    // Look for a free slot in main memory.
    auto* frame_table = self->frame_table();
    auto& free_map = self->free_map();
    std::uint8_t free_slot = std::countr_zero(free_map);

    // All frames are busy; swapping required.
    if (free_slot >= 16) {
      assert(free_map == 0);

      // Find slot in secondary memory to swap out the victim.
      auto* next_region = rebind<RegionPointer>(self->secondary_memory);
      if (next_region->length() == 0) {
        throw std::bad_alloc();
      }
      free_slot = swap_victim(self, next_region->offset());

      // Zero-initialize the fresh frame.
      std::fill_n(self->main_memory + (free_slot << 8), 256, std::byte{0});

      // Update the free list of secondary memory.
      next_region->offset() += 1;
      next_region->length() -= 1;
    }

    // Update the frame table.
    frame_table[free_slot].reset();
    frame_table[free_slot].set_referenced(true);

    // Update the free map.
    free_map = free_map & ~(1 << free_slot);

    // Update the translation table.
    PageEntry* pte = nullptr;
    auto* m = self->main_memory;

    if (i == 2) {
      pte = rebind<PageEntry>(pda);
    } else {
      auto offset = self->kalloc(16);
      pte = rebind<PageEntry>(m + offset) + ((va.raw >> shifts[1]) & 0x7);
      if (i == 0) {
        auto o = self->kalloc(16);
        rebind<std::uint16_t>(m + o)[(va.raw >> shifts[0]) & 0x7] = offset;
        offset = o;
      }

      *pda = offset;
    }

    *pte = PageEntry{};
    pte->set_present(true);
    pte->set_protection(ps);
    pte->set_frame(free_slot);
    frame_table[free_slot].add_back_reference(self->pte_offset(pte));
  }

  /// Selects a page to evict, swaps its contents to secondary memory, and returns the index of the
  /// freed frame in main memory.
  static std::uint8_t swap_victim(Machine* self, std::uint16_t secondary_slot) {
    assert(self->free_map() == 0);

    // Look for a "victim", i.e., a frame not referenced since the last stealing pass.
    auto victim = find_victim(self);

    // Swap data.
    auto* p = rebind<std::uint32_t>(self->secondary_memory + (secondary_slot << 8));
    auto* q = rebind<std::uint32_t>(self->main_memory + (victim << 8));
    for (auto i = 0; i < 64; ++i) { std::swap(p[i], q[i]); }

    update_page_entries_after_swap(self, victim, secondary_slot);
    return victim;
  }

  /// Finds a page to evict.
  static std::uint8_t find_victim(Machine* self) {
    auto* frame_table = self->frame_table();
    std::uint8_t victim = 0xff;

    // Look for a "victim", i.e., a frame not referenced since the last stealing pass, favoring
    // candidates with fewer back references.
    for (std::uint8_t s = 0; s < 16; ++s) {
      if (frame_table[s].is_ready_for_eviction()) {
        auto const n = frame_table[s].back_reference_count();
        if (n == 0) {
          return s;
        } else if ((victim >= 0x80) || (n < frame_table[victim].back_reference_count())) {
          victim = s;
        }
      } else {
        frame_table[s].set_referenced(false);
        if ((victim == 0xff) && !frame_table[s].is_pinned()) {
          victim = 0x80 | s;
        }
      }
    }

    // Did we find a frame that wasn't referenced?
    if (victim < 0x80) {
      assert((victim & 0xf0) == 0);
      return victim;
    }

    // Did we find at least one frame that wasn't pinned?
    else if (victim != 0xff) {
      assert((victim & 0x70) == 0);
      return victim & 15;
    }

    // Out of luck.
    else {
      throw std::bad_alloc();
    }
  }

  /// Update the page table entries that are currently referring to `victim`, which is the index of
  /// the frame containing a page whose contents has been swapped out to `secondary_slot`.
  static void update_page_entries_after_swap(
    Machine* self, std::uint8_t victim, std::uint16_t secondary_slot
  ) {
    auto& frame = self->frame_table()[victim];
    auto const n = frame.back_reference_count();
    if (n > 2) {
      // Feature not implemented.
      std::abort();
    }

    for (auto i = 0; i < n; ++i) {
      auto& pte = *rebind<PageEntry>(self->main_memory + frame.back_references()[i]);
      assert(pte.is_present());

      self->tlb.invalidate(pte);

      // Unset the present bit and re-map
      pte.set_present(false);
      pte.set_frame(secondary_slot);
    }

    frame.clear_back_references();
  }

};

} // namespace mmu