struct G101 : Chip {
  G101(Board& board, Markup::Node& boardNode) : Chip(board) {
  }

  auto prgAddress(uint addr) const -> uint {
    switch(addr & 0xe000) {
    case 0x8000:
      if(prgMode == 1) return (0x1e << 13) | (addr & 0x1fff);
      return (prgBank[0] << 13) | (addr & 0x1fff);
    case 0xa000:
      return (prgBank[1] << 13) | (addr & 0x1fff);
    case 0xc000:
      if(prgMode == 0) return (0x1e << 13) | (addr & 0x1fff);
      return (prgBank[0] << 13) | (addr & 0x1fff);
    case 0xe000:
      return (0x1f << 13) | (addr & 0x1fff);
    }
    unreachable;
  }

  auto chrAddress(uint addr) const -> uint {
    return (chrBank[addr >> 10] << 10) | (addr & 0x03ff);
  }

  auto ciramAddress(uint addr) const -> uint {
    switch(mirror) {
    case 0: return (addr & 0x03ff) | ((addr & 0x0400) >> 0);
    case 1: return (addr & 0x03ff) | ((addr & 0x0800) >> 1);
    }
    unreachable;
  }

  auto regWrite(uint addr, uint8 data) -> void {
    switch(addr & 0xf000) {
    case 0x8000:
      prgBank[0] = data & 0x1f;
      break;
    case 0x9000:
      mirror = data & 0x01;
      prgMode = data & 0x02;
      break;
    case 0xa000:
      prgBank[1] = data & 0x1f;
      break;
    case 0xb000:
      chrBank[addr & 0x0007] = data;
      break;
    }
  }

  auto power(bool reset) -> void {
    if(!reset) {
      prgMode = 0;
      prgBank[0] = 0x00;
      prgBank[1] = 0x1e;
      chrBank[0] = 0;
      chrBank[1] = 0;
      chrBank[2] = 0;
      chrBank[3] = 0;
      chrBank[4] = 0;
      chrBank[5] = 0;
      chrBank[6] = 0;
      chrBank[7] = 0;
      mirror = 0;
    }
  }

  auto serialize(serializer& s) -> void {
    s.integer(prgMode);
    s.array(prgBank);
    s.array(chrBank);
    s.integer(mirror);
  }

  bool prgMode;
  uint5 prgBank[2];
  uint8 chrBank[8];
  bool mirror;
};