// See: http://wiki.nesdev.com/w/index.php/INES_Mapper_078
// Note: Used for both Cosmo Carrier and Holy Diver

struct JalecoJF16 : Board {
  JalecoJF16(Markup::Node& document) : Board(document) {
    submapper = std::stoi((std::string) document["game/board/submapper/id"].text(), nullptr);
  }

  auto readPRG(uint addr) -> uint8 {
    if((addr & 0xc000) == 0x8000) return prgrom.read((prgBank << 14) | (addr & 0x3fff));
    if((addr & 0xc000) == 0xc000) return prgrom.read((   0x0f << 14) | (addr & 0x3fff));
    return cpu.mdr();
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) {
      mirrorSelect = (data & 0x8) ? 1 : 0;
      prgBank = data & 0x0f;
      chrBank = data >> 4;
    }
  }

  auto readCHR(uint addr) -> uint8 {
    if(addr & 0x2000) {
      if(submapper != 3) return ppu.readCIRAM((mirrorSelect << 10) | (addr & 0x03ff)); 
      if(mirrorSelect == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.readCIRAM(addr & 0x07ff);
    }
    addr = (chrBank * 0x2000) + (addr & 0x1fff);
    return Board::readCHR(addr);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(submapper != 3) return ppu.writeCIRAM((mirrorSelect << 10) | (addr & 0x03ff), data); 
      if(mirrorSelect == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.writeCIRAM(addr & 0x07ff, data);
    }
    addr = (chrBank * 0x2000) + (addr & 0x1fff);
    Board::writeCHR(addr, data);
  }

  auto power() -> void {
    prgBank = 0;
    chrBank = 0;
    mirrorSelect = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    s.integer(prgBank);
    s.integer(chrBank);
    s.integer(mirrorSelect);
  }

  uint2 submapper;
  uint4 prgBank;
  uint4 chrBank;
  bool mirrorSelect;
};
