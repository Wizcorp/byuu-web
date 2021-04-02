template<> auto CPU::bytes<Byte>() -> uint { return 1; }
template<> auto CPU::bytes<Word>() -> uint { return 2; }
template<> auto CPU::bytes<Long>() -> uint { return 4; }

template<> auto CPU::bits<Byte>() -> uint { return  8; }
template<> auto CPU::bits<Word>() -> uint { return 16; }
template<> auto CPU::bits<Long>() -> uint { return 32; }

template<> auto CPU::lsb<Byte>() -> uint32 { return 1; }
template<> auto CPU::lsb<Word>() -> uint32 { return 1; }
template<> auto CPU::lsb<Long>() -> uint32 { return 1; }

template<> auto CPU::msb<Byte>() -> uint32 { return       0x80; }
template<> auto CPU::msb<Word>() -> uint32 { return     0x8000; }
template<> auto CPU::msb<Long>() -> uint32 { return 0x80000000; }

template<> auto CPU::mask<Byte>() -> uint32 { return       0xff; }
template<> auto CPU::mask<Word>() -> uint32 { return     0xffff; }
template<> auto CPU::mask<Long>() -> uint32 { return 0xffffffff; }

template<> auto CPU::clip<Byte>(uint32 data) -> uint32 { return  (uint8)data; }
template<> auto CPU::clip<Word>(uint32 data) -> uint32 { return (uint16)data; }
template<> auto CPU::clip<Long>(uint32 data) -> uint32 { return (uint32)data; }

template<> auto CPU::sign<Byte>(uint32 data) -> int32 { return  (int8)data; }
template<> auto CPU::sign<Word>(uint32 data) -> int32 { return (int16)data; }
template<> auto CPU::sign<Long>(uint32 data) -> int32 { return (int32)data; }
