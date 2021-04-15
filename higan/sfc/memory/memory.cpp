#include <sfc/sfc.hpp>

namespace higan::SuperFamicom {

Bus bus;

Bus::~Bus() {}

auto Bus::reset() -> void {
  for(auto id : range(256)) {
    reader[id].reset();
    writer[id].reset();
    counter[id] = 0;
  }

  if(lookup) delete[] lookup;
  if(target) delete[] target;

  lookup = new uint8 [16 * 1024 * 1024]();
  target = new uint32[16 * 1024 * 1024]();

  reader[0] = [](uint24, uint8 data) -> uint8 { return data; };
  writer[0] = [](uint24, uint8) -> void {};

  cpu.map();
  ppu.map();
}

auto Bus::map(
  const function<uint8 (uint24, uint8)>& read,
  const function<void  (uint24, uint8)>& write,
  const string& addr, uint size, uint base, uint mask,
  unsigned fastmode, uint8* fastptr
) -> uint {
  uint id = 1;
  while(counter[id]) {
    if(++id >= 256) return print("SFC error: bus map exhausted\n"), 0;
  }

  reader[id] = read;
  writer[id] = write;

  auto p = addr.split(":", 1L);
  auto banks = p(0).split(",");
  auto addrs = p(1).split(",");
  for(auto& bank : banks) {
    for(auto& addr : addrs) {
      auto bankRange = bank.split("-", 1L);
      auto addrRange = addr.split("-", 1L);
      uint bankLo = bankRange(0).hex();
      uint bankHi = bankRange(1, bankRange(0)).hex();
      uint addrLo = addrRange(0).hex();
      uint addrHi = addrRange(1, addrRange(0)).hex();

      for(uint bank = bankLo; bank <= bankHi; bank++) {
        if (fastmode > 0) {
          for(unsigned addr = addrLo&~fast_page_size_mask; addr<=addrHi; addr+=fast_page_size) {
            unsigned origpos = (bank << 16 | addr);
            unsigned fastoffset = origpos >> fast_page_size_bits;

            unsigned accesspos = reduce(origpos, mask);
            if(size) accesspos = base + mirror(accesspos, size - base);
            if (fastmode > Bus::FastModeNone) fast_read[fastoffset] = fastptr - origpos + accesspos;
            if (fastmode == Bus::FastModeReadWrite) fast_write[fastoffset] = fastptr - origpos + accesspos;
          }
        }

        for(uint addr = addrLo; addr <= addrHi; addr++) {
          uint pid = lookup[bank << 16 | addr];
          if(pid && --counter[pid] == 0) {
            reader[pid].reset();
            writer[pid].reset();
          }

          uint offset = reduce(bank << 16 | addr, mask);
          if(size) base = mirror(base, size);
          if(size) offset = base + mirror(offset, size - base);
          lookup[bank << 16 | addr] = id;
          target[bank << 16 | addr] = offset;
          counter[id]++;
        }
      }
    }
  }

  return id;
}

auto Bus::unmap(const string& addr) -> void {
  auto p = addr.split(":", 1L);
  auto banks = p(0).split(",");
  auto addrs = p(1).split(",");
  for(auto& bank : banks) {
    for(auto& addr : addrs) {
      auto bankRange = bank.split("-", 1L);
      auto addrRange = addr.split("-", 1L);
      uint bankLo = bankRange(0).hex();
      uint bankHi = bankRange(1, bankRange(0)).hex();
      uint addrLo = addrRange(0).hex();
      uint addrHi = addrRange(1, addrRange(1)).hex();

      for(uint bank = bankLo; bank <= bankHi; bank++) {
        for(uint addr = addrLo; addr <= addrHi; addr++) {
          uint pid = lookup[bank << 16 | addr];
          if(pid && --counter[pid] == 0) {
            reader[pid].reset();
            writer[pid].reset();
          }

          lookup[bank << 16 | addr] = 0;
          target[bank << 16 | addr] = 0;
        }
      }
    }
  }
}

}
