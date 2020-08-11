struct N108 : Chip {
  N108(Board& board, Markup::Node& document) : Chip(board) {
    string type = document["game/board/chip/type"].text();

    if(type.match("*108*")) revision = Revision::N108;
    if(type.match("*109*")) revision = Revision::N109;
    if(type.match("*118*")) revision = Revision::N118;
    if(type.match("*119*")) revision = Revision::N119;
  }

  auto addrPRG(uint addr) const -> uint {
    switch((addr >> 13) & 3) {
    case 0: return (prgBank[0] << 13) | (addr & 0x1fff);
    case 1: return (prgBank[1] << 13) | (addr & 0x1fff);
    case 2: return (0x0e << 13) | (addr & 0x1fff);
    case 3: return (0x0f << 13) | (addr & 0x1fff);
    }
    unreachable;
  }

  auto addrCHR(uint addr) const -> uint {
    if(addr <= 0x07ff) return (chrBank[0] << 10) | (addr & 0x07ff);
    if(addr <= 0x0fff) return (chrBank[1] << 10) | (addr & 0x07ff);
    if(addr <= 0x13ff) return (chrBank[2] << 10) | (addr & 0x03ff);
    if(addr <= 0x17ff) return (chrBank[3] << 10) | (addr & 0x03ff);
    if(addr <= 0x1bff) return (chrBank[4] << 10) | (addr & 0x03ff);
    if(addr <= 0x1fff) return (chrBank[5] << 10) | (addr & 0x03ff);
    unreachable;
  }

  auto regWrite(uint addr, uint8 data) -> void {
    switch(addr & 0x8001) {
    case 0x8000:
      bankSelect = data & 0x07;
      break;

    case 0x8001:
      switch(bankSelect) {
      case 0: chrBank[0] = data & 0x3e; break;
      case 1: chrBank[1] = data & 0x3e; break;
      case 2: chrBank[2] = data & 0x3f; break;
      case 3: chrBank[3] = data & 0x3f; break;
      case 4: chrBank[4] = data & 0x3f; break;
      case 5: chrBank[5] = data & 0x3f; break;
      case 6: prgBank[0] = data & 0x0f; break;
      case 7: prgBank[1] = data & 0x0f; break;
      }
      break;
    }
  }

  auto power(bool reset) -> void {
    bankSelect = 0;
    prgBank[0] = 0;
    prgBank[1] = 0;
    chrBank[0] = 0;
    chrBank[1] = 0;
    chrBank[2] = 0;
    chrBank[3] = 0;
    chrBank[4] = 0;
    chrBank[5] = 0;
  }

  auto serialize(serializer& s) -> void {
    s.integer(bankSelect);
    s.array(prgBank);
    s.array(chrBank);
  }

  enum class Revision : uint {
    N108,
    N109,
    N118,
    N119,
  } revision;

  uint3 bankSelect;
  uint8 prgBank[2];
  uint8 chrBank[6];
};