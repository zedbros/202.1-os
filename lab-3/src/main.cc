#include "mmu.hh"

int main() {
  using namespace mmu;

  Machine m;
  PhysicalAddress pa;

  // The following 3 virtual addresses are already mapped. Because they belong to the same page,
  // only the first address translation requires a page walk. In more details, the first call
  // results in a TLB miss whereas the others result in TLB hits.
  std::cout << m.translate(0xf800, PageEntry::read) << std::endl;
  std::cout << m.translate(0xf810, PageEntry::read) << std::endl;
  std::cout << m.translate(0xf820, PageEntry::read) << std::endl;

  // The following virtual address is not mapped. Address translation raises a segfault.
  try {
    std::cout << m.translate(0x1000, PageEntry::read) << std::endl;
  } catch (PageLookupError const& e) {
    assert(e.cause == SegmentationFault);
  }

  // The following virtual address is not mapped. Address translation raises a segfault that gets
  // caught immediately to map the address to a newly allocate frame. This strategy is sometimes
  // used to implement `mmap`.
  std::cout << m.allocate_page(0x0800, PageEntry::read | PageEntry::write) << std::endl;

  // Now that `0x0800` is mapped, any address within the same page can be translated.
  std::cout << m.translate(0x0830, PageEntry::read) << std::endl;
  std::cout << m.translate(0x0840, PageEntry::read) << std::endl;

  pa = m.translate(0x0850, PageEntry::read | PageEntry::write);
  m.store_byte(std::byte{0x61}, pa);
  assert(m.read_byte(pa) == std::byte{0x61});

  // The following allocations will fill up main memory.
  for (auto i = 0; i < 14; ++i) {
    auto const a = 0x2000 + (i << 8);
    std::cout << m.allocate_page(a, PageEntry::read | PageEntry::write) << std::endl;
  }

  // This next allocation will cause swapping.
  std::cout << m.allocate_page(0x3000, PageEntry::read | PageEntry::write) << std::endl;

  // Given the current implementation, we can assume that the page that got evicted was mapped to
  // 0x0100, the frame in which we have written something. Since new pages are zeroed-out, we can
  // now expect to read a 0 from the same location.
  pa = m.translate(0x3050, PageEntry::read | PageEntry::write);
  assert(m.read_byte(pa) == std::byte{0});

  // The following translation causes the victim that has been swapped out to be swapped back into
  // main memory, but possibly at a different physical address.
  pa = m.translate(0x0850, PageEntry::read | PageEntry::write);
  assert(m.read_byte(pa) == std::byte{0x61});

  return 0;
}
