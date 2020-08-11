// http://wiki.nesdev.com/w/index.php/INES_Mapper_093

struct Sunsoft2 : Board {
  Sunsoft2(Markup::Node& document) : Board(document) {
    settings.mirror = document["game/board/mirror/mode"].text() == "vertical" ? 1 : 0;
  }

  auto readPRG(uint addr) -> uint8 {
    if((addr & 0xc000) == 0x8000) return prgrom.read((prgBank << 14) | (addr & 0x3fff));
    if((addr & 0xc000) == 0xc000) return prgrom.read((   0x0f << 14) | (addr & 0x3fff));
    return cpu.mdr();
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) { 
      prgBank = data >> 4;
      chrBank = (data >> 4 & 0x8) | (data & 0x7);
    }
  }

  auto readCHR(uint addr) -> uint8 {
    if(addr & 0x2000) {
      if(settings.mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.readCIRAM(addr & 0x07ff);
    }
    addr = (chrBank * 0x2000) + (addr & 0x1fff);
    return Board::readCHR(addr);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(settings.mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.writeCIRAM(addr & 0x07ff, data);
    }
    addr = (chrBank * 0x2000) + (addr & 0x1fff);
    Board::writeCHR(addr, data);
  }

  auto power() -> void {
    prgBank = 0;
    chrBank = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    s.integer(prgBank);
    s.integer(chrBank);
  }

  struct Settings {
    bool mirror;  //0 = horizontal, 1 = vertical
  } settings;

  uint4 prgBank;
  uint2 chrBank;
};