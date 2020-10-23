#include <fc/fc.hpp>

namespace higan::Famicom {

Bus bus;

//$0000-07ff = RAM (2KB)
//$0800-1fff = RAM (mirror)
//$2000-2007 = PPU
//$2008-3fff = PPU (mirror)
//$4000-4017 = APU + I/O
//$4020-403f = FDS
//$4018-ffff = Cartridge

auto Bus::read(uint16 addr) -> uint8 {
  uint8 data = cartridge.readPRG(addr);
  if(addr <= 0x1fff) return cpu.readRAM(addr);
  if(addr <= 0x3fff) return ppu.readIO(addr);
  if(addr <= 0x4017) return cpu.readIO(addr);
  return data;
}

auto Bus::write(uint16 addr, uint8 data) -> void {
  cartridge.writePRG(addr, data);
  if(addr <= 0x1fff) return cpu.writeRAM(addr, data);
  if(addr <= 0x3fff) return ppu.writeIO(addr, data);
  if(addr <= 0x4017) return cpu.writeIO(addr, data);
}

}
