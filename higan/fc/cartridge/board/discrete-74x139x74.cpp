// http://wiki.nesdev.com/w/index.php/INES_Mapper_087

struct DISCRETE74x139x74 : Board {
  DISCRETE74x139x74(Markup::Node& document) : Board(document) {
    settings.mirror = document["game/board/mirror/mode"].text() == "vertical" ? 1 : 0;
  }

  ~DISCRETE74x139x74() {}

  auto readPRG(uint addr) -> uint8 {
    if(addr & 0x8000) return prgrom.read(addr);
    return cpu.mdr();
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr == 0x6000) {
      chrBank = (data >> 1 & 0x1) | (data << 1 & 0x2);
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
    chrBank = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    s.integer(chrBank);
  }

  struct Settings {
    bool mirror;  //0 = horizontal, 1 = vertical
  } settings;

  uint2 chrBank;
};
