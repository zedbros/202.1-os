#include "mmu.hh"
#include <boost/ut.hpp>

int main() {
  using namespace boost::ut;
  using namespace mmu;

  "translate"_test = [] {
    Machine m;
    PhysicalAddress pa;

    // Page walk.
    pa = m.translate(0xf824, PageEntry::read);
    expect(pa.raw == 0x0024);

    // TLB hit.
    pa = m.translate(0xf834, PageEntry::read);
    expect(pa.raw == 0x0034);

    // Segfault.
    expect(throws([&] { m.translate(0x1000, PageEntry::read); }));
  };

  "mmap_simple"_test = [] {
    Machine m;
    auto const va = m.simple_mmap(0, 128, PageEntry::read | PageEntry::write);
    for (auto i = 0; i < 128; ++i) {
      auto const pa = m.translate(va.advanced(i), PageEntry::write);
      m.store_byte(std::byte{i & 0xff}, pa);
    }
    for (auto i = 0; i < 128; ++i) {
      auto const pa = m.translate(va.advanced(i), PageEntry::read);
      expect(m.read_byte(pa) == std::byte{i & 0xff});
    }
  };

  "mmap_protection"_test = [] {
    Machine m;
    auto const va = m.simple_mmap(0, 256, PageEntry::read | PageEntry::write);
    expect(throws([&] { m.translate(va, PageEntry::execute); }));
  };

  "mmap_large"_test = [] {
    Machine m;
    auto const va = m.simple_mmap(0, 8192, PageEntry::read | PageEntry::write);
    for (auto i = 0; i < 8192; ++i) {
      auto const pa = m.translate(va.advanced(i), PageEntry::write);
      m.store_byte(std::byte{i & 0xff}, pa);
    }
    for (auto i = 0; i < 8192; ++i) {
      auto const pa = m.translate(va.advanced(i), PageEntry::read);
      expect(m.read_byte(pa) == std::byte{i & 0xff});
    }
  };

  return 0;
}
