auto APU::ADD(uint8 x, uint8 y, bool c) -> uint8 {
  uint9 z = x + y + c;

  CF = z.bit(8);
  NF = 0;
  VF = uint8(~(x ^ y) & (x ^ z)).bit(7);
  XF = z.bit(3);
  HF = uint8(x ^ y ^ z).bit(4);
  YF = z.bit(5);
  ZF = uint8(z) == 0;
  SF = z.bit(7);

  return z;
}

auto APU::AND(uint8 x, uint8 y) -> uint8 {
  uint8 z = x & y;

  CF = 0;
  NF = 0;
  PF = parity(z);
  XF = z.bit(3);
  HF = 1;
  YF = z.bit(5);
  ZF = z == 0;
  SF = z.bit(7);

  return z;
}

auto APU::BIT(uint3 bit, uint8 x) -> uint8 {
  uint8 z = x & 1 << bit;

  NF = 0;
  PF = parity(z);
  XF = x.bit(3);
  HF = 1;
  YF = x.bit(5);
  ZF = z == 0;
  SF = z.bit(7);

  return x;
}

auto APU::CP(uint8 x, uint8 y) -> void {
  uint9 z = x - y;

  CF = z.bit(8);
  NF = 1;
  VF = uint8((x ^ y) & (x ^ z)).bit(7);
  XF = y.bit(3);
  HF = uint8(x ^ y ^ z).bit(4);
  YF = y.bit(5);
  ZF = uint8(z) == 0;
  SF = z.bit(7);
}

auto APU::DEC(uint8 x) -> uint8 {
  uint8 z = x - 1;

  NF = 1;
  VF = z == 0x7f;
  XF = z.bit(3);
  HF = z.bit(0,3) == 0x0f;
  YF = z.bit(5);
  ZF = z == 0;
  SF = z.bit(7);

  return z;
}

auto APU::IN(uint8 x) -> uint8 {
  NF = 0;
  PF = parity(x);
  XF = x.bit(3);
  HF = 0;
  YF = x.bit(5);
  ZF = x == 0;
  SF = x.bit(7);

  return x;
}

auto APU::INC(uint8 x) -> uint8 {
  uint8 z = x + 1;

  NF = 0;
  VF = z == 0x80;
  XF = z.bit(3);
  HF = z.bit(0,3) == 0x00;
  YF = z.bit(5);
  ZF = z == 0;
  SF = z.bit(7);

  return z;
}

auto APU::OR(uint8 x, uint8 y) -> uint8 {
  uint8 z = x | y;

  CF = 0;
  NF = 0;
  PF = parity(z);
  XF = z.bit(3);
  HF = 0;
  YF = z.bit(5);
  ZF = z == 0;
  SF = z.bit(7);

  return z;
}

auto APU::RES(uint3 bit, uint8 x) -> uint8 {
  x &= ~(1 << bit);
  return x;
}

auto APU::RL(uint8 x) -> uint8 {
  bool c = x.bit(7);
  x = x << 1 | CF;

  CF = c;
  NF = 0;
  PF = parity(x);
  XF = x.bit(3);
  HF = 0;
  YF = x.bit(5);
  ZF = x == 0;
  SF = x.bit(7);

  return x;
}

auto APU::RLC(uint8 x) -> uint8 {
  x = x << 1 | x >> 7;

  CF = x.bit(0);
  NF = 0;
  PF = parity(x);
  XF = x.bit(3);
  HF = 0;
  YF = x.bit(5);
  ZF = x == 0;
  SF = x.bit(7);

  return x;
}

auto APU::RR(uint8 x) -> uint8 {
  bool c = x.bit(0);
  x = x >> 1 | CF << 7;

  CF = c;
  NF = 0;
  PF = parity(x);
  XF = x.bit(3);
  HF = 0;
  YF = x.bit(5);
  ZF = x == 0;
  SF = x.bit(7);

  return x;
}

auto APU::RRC(uint8 x) -> uint8 {
  x = x >> 1 | x << 7;

  CF = x.bit(7);
  NF = 0;
  PF = parity(x);
  XF = x.bit(3);
  HF = 0;
  YF = x.bit(5);
  ZF = x == 0;
  SF = x.bit(7);

  return x;
}

auto APU::SET(uint3 bit, uint8 x) -> uint8 {
  x |= (1 << bit);
  return x;
}

auto APU::SLA(uint8 x) -> uint8 {
  bool c = x.bit(7);
  x = x << 1;

  CF = c;
  NF = 0;
  PF = parity(x);
  XF = x.bit(3);
  HF = 0;
  YF = x.bit(5);
  ZF = x == 0;
  SF = x.bit(7);

  return x;
}

auto APU::SLL(uint8 x) -> uint8 {
  bool c = x.bit(7);
  x = x << 1 | 1;

  CF = c;
  NF = 0;
  PF = parity(x);
  XF = x.bit(3);
  HF = 0;
  YF = x.bit(5);
  ZF = x == 0;
  SF = x.bit(7);

  return x;
}

auto APU::SRA(uint8 x) -> uint8 {
  bool c = x.bit(0);
  x = (int8)x >> 1;

  CF = c;
  NF = 0;
  PF = parity(x);
  XF = x.bit(3);
  HF = 0;
  YF = x.bit(5);
  ZF = x == 0;
  SF = x.bit(7);

  return x;
}

auto APU::SRL(uint8 x) -> uint8 {
  bool c = x.bit(0);
  x = x >> 1;

  CF = c;
  NF = 0;
  PF = parity(x);
  XF = x.bit(3);
  HF = 0;
  YF = x.bit(5);
  ZF = x == 0;
  SF = x.bit(7);

  return x;
}

auto APU::SUB(uint8 x, uint8 y, bool c) -> uint8 {
  uint9 z = x - y - c;

  CF = z.bit(8);
  NF = 1;
  VF = uint8((x ^ y) & (x ^ z)).bit(7);
  XF = z.bit(3);
  HF = uint8(x ^ y ^ z).bit(4);
  YF = z.bit(5);
  ZF = uint8(z) == 0;
  SF = z.bit(7);

  return z;
}

auto APU::XOR(uint8 x, uint8 y) -> uint8 {
  uint8 z = x ^ y;

  CF = 0;
  NF = 0;
  PF = parity(z);
  XF = z.bit(3);
  HF = 0;
  YF = z.bit(5);
  ZF = z == 0;
  SF = z.bit(7);

  return z;
}
