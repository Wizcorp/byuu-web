template<> auto CPU::_read<Byte>(uint32 address) -> uint32 {
  if(address & 1) {
    return read(0, 1, address & ~1).byte(0);
  } else {
    return read(1, 0, address & ~1).byte(1);
  }
}

template<> auto CPU::_read<Word>(uint32 address) -> uint32 {
  return read(1, 1, address & ~1);
}

template<> auto CPU::_read<Long>(uint32 address) -> uint32 {
  uint32 data = _read<Word>(address + 0) << 16;
  return data | _read<Word>(address + 2) <<  0;
}

template<uint Size> auto CPU::_readPC() -> uint32 {
  auto data = _read<Size == Byte ? Word : Size>(_pc);
  _pc += Size == Long ? 4 : 2;
  return clip<Size>(data);
}

auto CPU::_readDisplacement(uint32 base) -> uint32 {
  return base + (int16)_readPC<Word>();
}

auto CPU::_readIndex(uint32 base) -> uint32 {
  auto extension = _readPC<Word>();
  auto index = extension & 0x8000
  ? read(AddressRegister{extension >> 12})
  : read(DataRegister{extension >> 12});
  if(!(extension & 0x800)) index = (int16)index;
  return base + index + (int8)extension;
}

auto CPU::_dataRegister(DataRegister dr) -> string {
  return {"d", dr.number};
}

auto CPU::_addressRegister(AddressRegister ar) -> string {
  return {"a", ar.number};
}

template<uint Size> auto CPU::_immediate() -> string {
  return {"#$", hex(_readPC<Size>(), 2 << Size)};
}

template<uint Size> auto CPU::_address(EffectiveAddress& ea) -> string {
  if(ea.mode ==  2) return {_addressRegister(AddressRegister{ea.reg})};
  if(ea.mode ==  5) return {"$", hex(_readDisplacement(read(AddressRegister{ea.reg})), 6L)};
  if(ea.mode ==  6) return {"$", hex(_readIndex(read(AddressRegister{ea.reg})), 6L)};
  if(ea.mode ==  7) return {"$", hex((int16)_readPC<Word>(), 6L)};
  if(ea.mode ==  8) return {"$", hex(_readPC<Long>(), 6L)};
  if(ea.mode ==  9) return {"$", hex(_pc + (int16)_readPC(), 6L)};
  if(ea.mode == 10) return {"$", hex(_readIndex(_pc), 6L)};
  return "???";  //should never occur (modes 0, 1, 3, 4, 11 are not valid for LEA)
}

template<uint Size> auto CPU::_effectiveAddress(EffectiveAddress& ea) -> string {
  if(ea.mode ==  0) return {_dataRegister(DataRegister{ea.reg})};
  if(ea.mode ==  1) return {_addressRegister(AddressRegister{ea.reg})};
  if(ea.mode ==  2) return {"(", _addressRegister(AddressRegister{ea.reg}), ")"};
  if(ea.mode ==  3) return {"(", _addressRegister(AddressRegister{ea.reg}), ")+"};
  if(ea.mode ==  4) return {"-(", _addressRegister(AddressRegister{ea.reg}), ")"};
  if(ea.mode ==  5) return {"($", hex(_readDisplacement(read(AddressRegister{ea.reg})), 6L), ")"};
  if(ea.mode ==  6) return {"($", hex(_readIndex(read(AddressRegister{ea.reg})), 6L), ")"};
  if(ea.mode ==  7) return {"($", hex((int16)_readPC<Word>(), 6L), ")"};
  if(ea.mode ==  8) return {"($", hex(_readPC<Long>(), 6L), ")"};
  if(ea.mode ==  9) return {"($", hex(_readDisplacement(_pc), 6L), ")"};
  if(ea.mode == 10) return {"($", hex(_readIndex(_pc), 6L), ")"};
  if(ea.mode == 11) return {"#$", hex(_readPC<Size>(), 2 << Size)};
  return "???";  //should never occur
}

auto CPU::_branch(uint8 displacement) -> string {
  uint16 extension = _readPC();
  _pc -= 2;
  int32 offset = displacement ? sign<Byte>(displacement) : sign<Word>(extension);
  return {"$", hex(_pc + offset, 6L)};
}

template<uint Size> auto CPU::_suffix() -> string {
  return Size == Byte ? ".b" : Size == Word ? ".w" : ".l";
}

auto CPU::_condition(uint4 condition) -> string {
  static const string conditions[16] = {
    "t ", "f ", "hi", "ls", "cc", "cs", "ne", "eq",
    "vc", "vs", "pl", "mi", "ge", "lt", "gt", "le",
  };
  return conditions[condition];
}

auto CPU::disassembleInstruction(uint32 pc) -> string {
  _pc = pc;
  return pad(disassembleTable[_readPC()](), -60);  //todo: exact maximum length unknown (and sub-optimal)
}

auto CPU::disassembleContext() -> string {
  return {
    "d0:", hex(r.d[0], 8L), " ",
    "d1:", hex(r.d[1], 8L), " ",
    "d2:", hex(r.d[2], 8L), " ",
    "d3:", hex(r.d[3], 8L), " ",
    "d4:", hex(r.d[4], 8L), " ",
    "d5:", hex(r.d[5], 8L), " ",
    "d6:", hex(r.d[6], 8L), " ",
    "d7:", hex(r.d[7], 8L), " ",
    "a0:", hex(r.a[0], 8L), " ",
    "a1:", hex(r.a[1], 8L), " ",
    "a2:", hex(r.a[2], 8L), " ",
    "a3:", hex(r.a[3], 8L), " ",
    "a4:", hex(r.a[4], 8L), " ",
    "a5:", hex(r.a[5], 8L), " ",
    "a6:", hex(r.a[6], 8L), " ",
    "a7:", hex(r.a[7], 8L), " ",
    "sp:", hex(r.sp,   8L), " ",
    r.t ? "T" : "t",
    r.s ? "S" : "s",
    (uint)r.i,
    r.c ? "C" : "c",
    r.v ? "V" : "v",
    r.z ? "Z" : "z",
    r.n ? "N" : "n",
    r.x ? "X" : "x", " ",
  };
}

//

auto CPU::disassembleABCD(EffectiveAddress from, EffectiveAddress with) -> string {
  return {"abcd    ", _effectiveAddress<Byte>(from), ",", _effectiveAddress<Byte>(with)};
}

template<uint Size> auto CPU::disassembleADD(EffectiveAddress from, DataRegister with) -> string {
  return {"add", _suffix<Size>(), "   ", _effectiveAddress<Size>(from), ",", _dataRegister(with)};
}

template<uint Size> auto CPU::disassembleADD(DataRegister from, EffectiveAddress with) -> string {
  return {"add", _suffix<Size>(), "   ", _dataRegister(from), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleADDA(EffectiveAddress from, AddressRegister with) -> string {
  return {"adda", _suffix<Size>(), "  ", _effectiveAddress<Size>(from), ",", _addressRegister(with)};
}

template<uint Size> auto CPU::disassembleADDI(EffectiveAddress with) -> string {
  return {"addi", _suffix<Size>(), "  ", _immediate<Size>(), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleADDQ(uint4 immediate, EffectiveAddress with) -> string {
  return {"addq", _suffix<Size>(), "  #", immediate, ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleADDQ(uint4 immediate, AddressRegister with) -> string {
  return {"addq", _suffix<Size>(), "  #", immediate, ",", _addressRegister(with)};
}

template<uint Size> auto CPU::disassembleADDX(EffectiveAddress from, EffectiveAddress with) -> string {
  return {"addx", _suffix<Size>(), "  ", _effectiveAddress<Size>(from), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleAND(EffectiveAddress from, DataRegister with) -> string {
  return {"and", _suffix<Size>(), "   ", _effectiveAddress<Size>(from), ",", _dataRegister(with)};
}

template<uint Size> auto CPU::disassembleAND(DataRegister from, EffectiveAddress with) -> string {
  return {"and", _suffix<Size>(), "   ", _dataRegister(from), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleANDI(EffectiveAddress ea) -> string {
  return {"andi", _suffix<Size>(), "  ", _immediate<Size>(), ",", _effectiveAddress<Size>(ea)};
}

auto CPU::disassembleANDI_TO_CCR() -> string {
  return {"andi    ", _immediate<Byte>(), ",ccr"};
}

auto CPU::disassembleANDI_TO_SR() -> string {
  return {"andi    ", _immediate<Word>(), ",sr"};
}

template<uint Size> auto CPU::disassembleASL(uint4 count, DataRegister with) -> string {
  return {"asl", _suffix<Size>(), "   #", count, ",", _dataRegister(with)};
}

template<uint Size> auto CPU::disassembleASL(DataRegister from, DataRegister with) -> string {
  return {"asl", _suffix<Size>(), "   ", _dataRegister(from), ",", _dataRegister(with)};
}

auto CPU::disassembleASL(EffectiveAddress with) -> string {
  return {"asl", _suffix<Word>(), "   ", _effectiveAddress<Word>(with)};
}

template<uint Size> auto CPU::disassembleASR(uint4 count, DataRegister modify) -> string {
  return {"asr", _suffix<Size>(), "   #", count, ",", _dataRegister(modify)};
}

template<uint Size> auto CPU::disassembleASR(DataRegister from, DataRegister modify) -> string {
  return {"asr", _suffix<Size>(), "   ", _dataRegister(from), ",", _dataRegister(modify)};
}

auto CPU::disassembleASR(EffectiveAddress with) -> string {
  return {"asr", _suffix<Word>(), "   ", _effectiveAddress<Word>(with)};
}

auto CPU::disassembleBCC(uint4 test, uint8 displacement) -> string {
  auto cc = _condition(test);
  return {"b", cc, "     ", _branch(displacement)};
}

template<uint Size> auto CPU::disassembleBCHG(DataRegister bit, EffectiveAddress with) -> string {
  return {"bchg", _suffix<Size>(), "  ", _dataRegister(bit), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleBCHG(EffectiveAddress with) -> string {
  return {"bchg", _suffix<Size>(), "  ", _immediate<Byte>(), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleBCLR(DataRegister bit, EffectiveAddress with) -> string {
  return {"bclr", _suffix<Size>(), "  ", _dataRegister(bit), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleBCLR(EffectiveAddress with) -> string {
  return {"bclr", _suffix<Size>(), "  ", _immediate<Byte>(), ",", _effectiveAddress<Size>(with)};
}

auto CPU::disassembleBRA(uint8 displacement) -> string {
  return {"bra     ", _branch(displacement)};
}

template<uint Size> auto CPU::disassembleBSET(DataRegister bit, EffectiveAddress with) -> string {
  return {"bset", _suffix<Size>(), "  ", _dataRegister(bit), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleBSET(EffectiveAddress with) -> string {
  return {"bset", _suffix<Size>(), "  ", _immediate<Byte>(), ",", _effectiveAddress<Size>(with)};
}

auto CPU::disassembleBSR(uint8 displacement) -> string {
  return {"bsr     ", _branch(displacement)};
}

template<uint Size> auto CPU::disassembleBTST(DataRegister bit, EffectiveAddress with) -> string {
  return {"btst", _suffix<Size>(), "  ", _dataRegister(bit), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleBTST(EffectiveAddress with) -> string {
  return {"btst", _suffix<Size>(), "  ", _immediate<Byte>(), ",", _effectiveAddress<Size>(with)};
}

auto CPU::disassembleCHK(DataRegister compare, EffectiveAddress maximum) -> string {
  return {"chk", _suffix<Word>(), "   ", _effectiveAddress<Word>(maximum), ",", _dataRegister(compare)};
}

template<uint Size> auto CPU::disassembleCLR(EffectiveAddress ea) -> string {
  return {"clr", _suffix<Size>(), "   ", _effectiveAddress<Size>(ea)};
}

template<uint Size> auto CPU::disassembleCMP(EffectiveAddress from, DataRegister with) -> string {
  return {"cmp", _suffix<Size>(), "   ", _effectiveAddress<Size>(from), ",", _dataRegister(with)};
}

template<uint Size> auto CPU::disassembleCMPA(EffectiveAddress from, AddressRegister with) -> string {
  return {"cmpa", _suffix<Size>(), "  ", _effectiveAddress<Size>(from), ",", _addressRegister(with)};
}

template<uint Size> auto CPU::disassembleCMPI(EffectiveAddress with) -> string {
  return {"cmpi", _suffix<Size>(), "  ", _immediate<Size>(), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleCMPM(EffectiveAddress from, EffectiveAddress with) -> string {
  return {"cmpm", _suffix<Size>(), "  ", _effectiveAddress<Size>(from), ",", _effectiveAddress<Size>(with)};
}

auto CPU::disassembleDBCC(uint4 condition, DataRegister with) -> string {
  auto base = _pc;
  auto displacement = (int16)_readPC();
  return {"db", _condition(condition), "    ", _dataRegister(with), ",$", hex(base + displacement, 6L)};
}

auto CPU::disassembleDIVS(EffectiveAddress from, DataRegister with) -> string {
  return {"divs", _suffix<Word>(), "  ", _effectiveAddress<Word>(from), ",", _dataRegister(with)};
}

auto CPU::disassembleDIVU(EffectiveAddress from, DataRegister with) -> string {
  return {"divu", _suffix<Word>(), "  ", _effectiveAddress<Word>(from), ",", _dataRegister(with)};
}

template<uint Size> auto CPU::disassembleEOR(DataRegister from, EffectiveAddress with) -> string {
  return {"eor", _suffix<Size>(), "   ", _dataRegister(from), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleEORI(EffectiveAddress with) -> string {
  return {"eori", _suffix<Size>(), "  ", _immediate<Size>(), ",", _effectiveAddress<Size>(with)};
}

auto CPU::disassembleEORI_TO_CCR() -> string {
  return {"eori    ", _immediate<Byte>(), ",ccr"};
}

auto CPU::disassembleEORI_TO_SR() -> string {
  return {"eori    ", _immediate<Word>(), ",sr"};
}

auto CPU::disassembleEXG(DataRegister x, DataRegister y) -> string {
  return {"exg     ", _dataRegister(x), ",", _dataRegister(y)};
}

auto CPU::disassembleEXG(AddressRegister x, AddressRegister y) -> string {
  return {"exg     ", _addressRegister(x), ",", _addressRegister(y)};
}

auto CPU::disassembleEXG(DataRegister x, AddressRegister y) -> string {
  return {"exg     ", _dataRegister(x), ",", _addressRegister(y)};
}

template<uint Size> auto CPU::disassembleEXT(DataRegister with) -> string {
  return {"ext", _suffix<Size>(), "   ", _dataRegister(with)};
}

auto CPU::disassembleILLEGAL(uint16 code) -> string {
  if(code.bit(12,15) == 0xa) return {"linea   $", hex(code.bit(0,11), 3L)};
  if(code.bit(12,15) == 0xf) return {"linef   $", hex(code.bit(0,11), 3L)};
  return {"illegal "};
}

auto CPU::disassembleJMP(EffectiveAddress from) -> string {
  return {"jmp     ", _effectiveAddress<Long>(from)};
}

auto CPU::disassembleJSR(EffectiveAddress from) -> string {
  return {"jsr     ", _effectiveAddress<Long>(from)};
}

auto CPU::disassembleLEA(EffectiveAddress from, AddressRegister with) -> string {
  return {"lea     ", _address<Long>(from), ",", _addressRegister(with)};
}

auto CPU::disassembleLINK(AddressRegister with) -> string {
  return {"link    ", _addressRegister(with), ",", _immediate<Word>()};
}

template<uint Size> auto CPU::disassembleLSL(uint4 count, DataRegister with) -> string {
  return {"lsl", _suffix<Size>(), "   #", count, ",", _dataRegister(with)};
}

template<uint Size> auto CPU::disassembleLSL(DataRegister from, DataRegister with) -> string {
  return {"lsl", _suffix<Size>(), "   ", _dataRegister(from), ",", _dataRegister(with)};
}

auto CPU::disassembleLSL(EffectiveAddress with) -> string {
  return {"lsl", _suffix<Word>(), "   ", _effectiveAddress<Word>(with)};
}

template<uint Size> auto CPU::disassembleLSR(uint4 count, DataRegister with) -> string {
  return {"lsr", _suffix<Size>(), "   #", count, ",", _dataRegister(with)};
}

template<uint Size> auto CPU::disassembleLSR(DataRegister from, DataRegister with) -> string {
  return {"lsr", _suffix<Size>(), "   ", _dataRegister(from), ",", _dataRegister(with)};
}

auto CPU::disassembleLSR(EffectiveAddress with) -> string {
  return {"lsr", _suffix<Word>(), "   ", _effectiveAddress<Word>(with)};
}

template<uint Size> auto CPU::disassembleMOVE(EffectiveAddress from, EffectiveAddress to) -> string {
  return {"move", _suffix<Size>(), "  ", _effectiveAddress<Size>(from), ",", _effectiveAddress<Size>(to)};
}

template<uint Size> auto CPU::disassembleMOVEA(EffectiveAddress from, AddressRegister to) -> string {
  return {"movea   ", _effectiveAddress<Size>(from), ",", _addressRegister(to)};
}

template<uint Size> auto CPU::disassembleMOVEM_TO_MEM(EffectiveAddress to) -> string {
  string op{"movem", _suffix<Size>(), " "};

  uint16 list = _readPC();
  string regs;
  for(uint n : range(8)) if(list.bit(0 + n)) regs.append(_dataRegister(DataRegister{n}), ",");
  regs.trimRight(",");
  if(regs && list >> 8) regs.append("/");
  for(uint n : range(8)) if(list.bit(8 + n)) regs.append(_addressRegister(AddressRegister{n}), ",");
  regs.trimRight(",");

  return {op, regs, ",", _effectiveAddress<Size>(to)};
}

template<uint Size> auto CPU::disassembleMOVEM_TO_REG(EffectiveAddress from) -> string {
  string op{"movem", _suffix<Size>(), " "};

  uint16 list = _readPC();
  string regs;
  for(uint n : range(8)) if(list.bit(0 + n)) regs.append(_dataRegister(DataRegister{n}), ",");
  regs.trimRight(",");
  if(regs && list >> 8) regs.append("/");
  for(uint n : range(8)) if(list.bit(8 + n)) regs.append(_addressRegister(AddressRegister{n}), ",");
  regs.trimRight(",");

  return {op, _effectiveAddress<Size>(from), ",", regs};
}

template<uint Size> auto CPU::disassembleMOVEP(DataRegister from, EffectiveAddress to) -> string {
  return {"movep", _suffix<Size>(), " ", _dataRegister(from), ",", _effectiveAddress<Size>(to)};
}

template<uint Size> auto CPU::disassembleMOVEP(EffectiveAddress from, DataRegister to) -> string {
  return {"movep", _suffix<Size>(), " ", _effectiveAddress<Size>(from), ",", _dataRegister(to)};
}

auto CPU::disassembleMOVEQ(uint8 immediate, DataRegister to) -> string {
  return {"moveq   #$", hex(immediate, 2L), ",", _dataRegister(to)};
}

auto CPU::disassembleMOVE_FROM_SR(EffectiveAddress to) -> string {
  return {"move    sr,", _effectiveAddress<Word>(to)};
}

auto CPU::disassembleMOVE_TO_CCR(EffectiveAddress from) -> string {
  return {"move    ", _effectiveAddress<Byte>(from), ",ccr"};
}

auto CPU::disassembleMOVE_TO_SR(EffectiveAddress from) -> string {
  return {"move    ", _effectiveAddress<Word>(from), ",sr"};
}

auto CPU::disassembleMOVE_FROM_USP(AddressRegister to) -> string {
  return {"move    usp,", _addressRegister(to)};
}

auto CPU::disassembleMOVE_TO_USP(AddressRegister from) -> string {
  return {"move    ", _addressRegister(from), ",usp"};
}

auto CPU::disassembleMULS(EffectiveAddress from, DataRegister with) -> string {
  return {"muls", _suffix<Word>(), "  ", _effectiveAddress<Word>(from), ",", _dataRegister(with)};
}

auto CPU::disassembleMULU(EffectiveAddress from, DataRegister with) -> string {
  return {"mulu", _suffix<Word>(), "  ", _effectiveAddress<Word>(from), ",", _dataRegister(with)};
}

auto CPU::disassembleNBCD(EffectiveAddress with) -> string {
  return {"nbcd    ", _effectiveAddress<Byte>(with)};
}

template<uint Size> auto CPU::disassembleNEG(EffectiveAddress with) -> string {
  return {"neg", _suffix<Size>(), "   ", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleNEGX(EffectiveAddress with) -> string {
  return {"negx", _suffix<Size>(), "  ", _effectiveAddress<Size>(with)};
}

auto CPU::disassembleNOP() -> string {
  return {"nop     "};
}

template<uint Size> auto CPU::disassembleNOT(EffectiveAddress with) -> string {
  return {"not", _suffix<Size>(), "   ", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleOR(EffectiveAddress from, DataRegister with) -> string {
  return {"or", _suffix<Size>(), "    ", _effectiveAddress<Size>(from), ",", _dataRegister(with)};
}

template<uint Size> auto CPU::disassembleOR(DataRegister from, EffectiveAddress with) -> string {
  return {"or", _suffix<Size>(), "    ", _dataRegister(from), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleORI(EffectiveAddress with) -> string {
  return {"ori", _suffix<Size>(), "   ", _immediate<Size>(), ",", _effectiveAddress<Size>(with)};
}

auto CPU::disassembleORI_TO_CCR() -> string {
  return {"ori     ", _immediate<Byte>(), ",ccr"};
}

auto CPU::disassembleORI_TO_SR() -> string {
  return {"ori     ", _immediate<Word>(), ",sr"};
}

auto CPU::disassemblePEA(EffectiveAddress from) -> string {
  return {"pea     ", _effectiveAddress<Long>(from)};
}

auto CPU::disassembleRESET() -> string {
  return {"reset   "};
}

template<uint Size> auto CPU::disassembleROL(uint4 count, DataRegister with) -> string {
  return {"rol", _suffix<Size>(), "   #", count, ",", _dataRegister(with)};
}

template<uint Size> auto CPU::disassembleROL(DataRegister from, DataRegister with) -> string {
  return {"rol", _suffix<Size>(), "   ", _dataRegister(from), ",", _dataRegister(with)};
}

auto CPU::disassembleROL(EffectiveAddress with) -> string {
  return {"rol", _suffix<Word>(), "   ", _effectiveAddress<Word>(with)};
}

template<uint Size> auto CPU::disassembleROR(uint4 count, DataRegister with) -> string {
  return {"ror", _suffix<Size>(), "   #", count, ",", _dataRegister(with)};
}

template<uint Size> auto CPU::disassembleROR(DataRegister from, DataRegister with) -> string {
  return {"ror", _suffix<Size>(), "   ", _dataRegister(from) ,",", _dataRegister(with)};
}

auto CPU::disassembleROR(EffectiveAddress with) -> string {
  return {"ror", _suffix<Word>(), "   ", _effectiveAddress<Word>(with)};
}

template<uint Size> auto CPU::disassembleROXL(uint4 count, DataRegister with) -> string {
  return {"roxl", _suffix<Size>(), "  #", count, ",", _dataRegister(with)};
}

template<uint Size> auto CPU::disassembleROXL(DataRegister from, DataRegister with) -> string {
  return {"roxl", _suffix<Size>(), "  ", _dataRegister(from), ",", _dataRegister(with)};
}

auto CPU::disassembleROXL(EffectiveAddress with) -> string {
  return {"roxl", _suffix<Word>(), "  ", _effectiveAddress<Word>(with)};
}

template<uint Size> auto CPU::disassembleROXR(uint4 count, DataRegister with) -> string {
  return {"roxr", _suffix<Size>(), "  #", count, ",", _dataRegister(with)};
}

template<uint Size> auto CPU::disassembleROXR(DataRegister from, DataRegister with) -> string {
  return {"roxr", _suffix<Size>(), "  ", _dataRegister(from), ",", _dataRegister(with)};
}

auto CPU::disassembleROXR(EffectiveAddress with) -> string {
  return {"roxr", _suffix<Word>(), "  ", _effectiveAddress<Word>(with)};
}

auto CPU::disassembleRTE() -> string {
  return {"rte     "};
}

auto CPU::disassembleRTR() -> string {
  return {"rtr     "};
}

auto CPU::disassembleRTS() -> string {
  return {"rts     "};
}

auto CPU::disassembleSBCD(EffectiveAddress from, EffectiveAddress with) -> string {
  return {"sbcd    ", _effectiveAddress<Byte>(from), ",", _effectiveAddress<Byte>(with)};
}

auto CPU::disassembleSCC(uint4 test, EffectiveAddress to) -> string {
  return {"s", _condition(test), "     ", _effectiveAddress<Byte>(to)};
}

auto CPU::disassembleSTOP() -> string {
  return {"stop    ", _immediate<Word>()};
}

template<uint Size> auto CPU::disassembleSUB(EffectiveAddress from, DataRegister with) -> string {
  return {"sub", _suffix<Size>(), "   ", _effectiveAddress<Size>(from), ",", _dataRegister(with)};
}

template<uint Size> auto CPU::disassembleSUB(DataRegister from, EffectiveAddress with) -> string {
  return {"sub", _suffix<Size>(), "   ", _dataRegister(from), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleSUBA(EffectiveAddress from, AddressRegister with) -> string {
  return {"suba", _suffix<Size>(), "  ", _effectiveAddress<Size>(from), ",", _addressRegister(with)};
}

template<uint Size> auto CPU::disassembleSUBI(EffectiveAddress with) -> string {
  return {"subi", _suffix<Size>(), "  ", _immediate<Size>(), ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleSUBQ(uint4 immediate, EffectiveAddress with) -> string {
  return {"subq", _suffix<Size>(), "  #", immediate, ",", _effectiveAddress<Size>(with)};
}

template<uint Size> auto CPU::disassembleSUBQ(uint4 immediate, AddressRegister with) -> string {
  return {"subq", _suffix<Size>(), "  #", immediate, ",", _addressRegister(with)};
}

template<uint Size> auto CPU::disassembleSUBX(EffectiveAddress from, EffectiveAddress with) -> string {
  return {"subx", _suffix<Size>(), "  ", _effectiveAddress<Size>(from), ",", _effectiveAddress<Size>(with)};
}

auto CPU::disassembleSWAP(DataRegister with) -> string {
  return {"swap    ", _dataRegister(with)};
}

auto CPU::disassembleTAS(EffectiveAddress with) -> string {
  return {"tas     ", _effectiveAddress<Byte>(with)};
}

auto CPU::disassembleTRAP(uint4 vector) -> string {
  return {"trap    #", vector};
}

auto CPU::disassembleTRAPV() -> string {
  return {"trapv   "};
}

template<uint Size> auto CPU::disassembleTST(EffectiveAddress from) -> string {
  return {"tst", _suffix<Size>(), "   ", _effectiveAddress<Size>(from)};
}

auto CPU::disassembleUNLK(AddressRegister with) -> string {
  return {"unlk    ", _addressRegister(with)};
}
