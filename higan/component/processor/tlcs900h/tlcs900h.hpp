//Toshiba TLCS900/H

/* open questions:
 *
 * what happens when a prohibited instruction operand size is used? (eg adc.l (memory),#immediate)
 * what happens when %11 is used for pre-decrement and post-increment addressing?
 * what happens when using 8-bit register indexing and d0 is set (Word) or d1/d0 is set (Long)?
 * what happens during an LDX instruction when the three padding bytes aren't all 0x00?
 * what value is read back from a non-existent 8-bit register ID? (eg 0x40-0xcf)
 * many instructions are undefined, some are marked as dummy instructions ... what do each do?
 * what happens when using (AND,OR,XOR,LD)CF (byte)source,A with A.bit(3) set?
 * what happens when using MINC#,MDEC# with a non-power of two? what about with a value of 1?
 */

#pragma once

namespace higan {

struct TLCS900H {
  enum : uint { Byte = 1, Word = 2, Long = 4 };

  virtual auto idle(uint clocks) -> void = 0;
  virtual auto width(uint24 address) -> uint = 0;
  virtual auto read(uint size, uint24 address) -> uint32 = 0;
  virtual auto write(uint size, uint24 address, uint32 data) -> void = 0;

  struct FlagRegister   { using type =  uint8; enum : uint { bits =  8 }; uint1 id; };
  struct StatusRegister { using type = uint16; enum : uint { bits = 16 }; };
  struct ProgramCounter { using type = uint32; enum : uint { bits = 32 }; };
  template<typename T> struct ControlRegister { using type = T; enum : uint { bits = 8 * sizeof(T) };  uint8 id; };
  template<typename T> struct Register        { using type = T; enum : uint { bits = 8 * sizeof(T) };  uint8 id; };
  template<typename T> struct Memory          { using type = T; enum : uint { bits = 8 * sizeof(T) }; uint32 address;  };
  template<typename T> struct Immediate       { using type = T; enum : uint { bits = 8 * sizeof(T) }; uint32 constant; };

  template<typename T> auto load(Immediate<T> immediate) const -> T { return immediate.constant; }

  //tlcs900h.cpp
  auto interrupt(uint8 vector) -> void;
  auto power() -> void;

  //registers.cpp
  template<typename T> auto map(Register<T>) const -> maybe<T&>;
  template<typename T> auto load(Register<T>) const -> T;
  template<typename T> auto store(Register<T>, uint32) -> void;
  auto expand(Register< uint8>) const -> Register<uint16>;
  auto expand(Register<uint16>) const -> Register<uint32>;
  auto shrink(Register<uint32>) const -> Register<uint16>;
  auto shrink(Register<uint16>) const -> Register< uint8>;
  auto load(FlagRegister) const -> uint8;
  auto store(FlagRegister, uint8) -> void;
  auto load(StatusRegister) const -> uint16;
  auto store(StatusRegister, uint16) -> void;
  auto load(ProgramCounter) const -> uint32;
  auto store(ProgramCounter, uint32) -> void;

  //control-registers.cpp
  template<typename T> auto map(ControlRegister<T>) const -> maybe<T&>;
  template<typename T> auto load(ControlRegister<T>) const -> T;
  template<typename T> auto store(ControlRegister<T>, uint32) -> void;

  //memory.cpp
  template<typename T = uint8> auto fetch() -> T;
  template<typename T> auto fetchRegister() -> Register<T>;
  template<typename T, typename U> auto fetchMemory() -> Memory<T>;
  template<typename T> auto fetchImmediate() -> Immediate<T>;
  template<typename T> auto push(T) -> void;
  template<typename T> auto pop(T) -> void;
  template<typename T> auto load(Memory<T>) -> T;
  template<typename T> auto store(Memory<T>, uint32) -> void;

  //conditions.cpp
  auto condition(uint4 code) -> bool;

  //algorithms.cpp
  template<typename T> auto parity(T) const -> bool;
  template<typename T> auto algorithmAdd(T target, T source, uint1 carry = 0) -> T;
  template<typename T> auto algorithmAnd(T target, T source) -> T;
  template<typename T> auto algorithmDecrement(T target, T source) -> T;
  template<typename T> auto algorithmIncrement(T target, T source) -> T;
  template<typename T> auto algorithmOr(T target, T source) -> T;
  template<typename T> auto algorithmRotated(T result) -> T;
  template<typename T> auto algorithmShifted(T result) -> T;
  template<typename T> auto algorithmSubtract(T target, T source, uint1 carry = 0) -> T;
  template<typename T> auto algorithmXor(T target, T source) -> T;

  //dma.cpp
  auto dma(uint2 channel) -> bool;

  //instruction.cpp
  template<uint Bits> auto idleBW(uint b, uint w) -> void;
  template<uint Bits> auto idleWL(uint w, uint l) -> void;
  template<uint Bits> auto idleBWL(uint b, uint w, uint l) -> void;

  template<typename T> auto toRegister3(uint3) const -> Register<T>;
  template<typename T> auto toRegister8(uint8) const -> Register<T>;
  template<typename T> auto toControlRegister(uint8) const -> ControlRegister<T>;
  template<typename T> auto toMemory(uint32 address) const -> Memory<T>;
  template<typename T> auto toImmediate(uint32 constant) const -> Immediate<T>;
  template<typename T> auto toImmediate3(natural constant) const -> Immediate<T>;
  auto undefined() -> void;
  auto instruction() -> void;
  template<typename Register> auto instructionRegister(Register) -> void;
  template<typename Memory> auto instructionSourceMemory(Memory) -> void;
  auto instructionTargetMemory(uint32 address) -> void;

  //instructions.cpp
  template<typename Target, typename Source> auto instructionAdd(Target, Source) -> void;
  template<typename Target, typename Source> auto instructionAddCarry(Target, Source) -> void;
  template<typename Target, typename Source> auto instructionAnd(Target, Source) -> void;
  template<typename Source, typename Offset> auto instructionAndCarry(Source, Offset) -> void;
  template<typename Source, typename Offset> auto instructionBit(Source, Offset) -> void;
  auto instructionBitSearch1Backward(Register<uint16>) -> void;
  auto instructionBitSearch1Forward(Register<uint16>) -> void;
  template<typename Source> auto instructionCall(Source) -> void;
  template<typename Source> auto instructionCallRelative(Source) -> void;
  template<typename Target, typename Offset> auto instructionChange(Target, Offset) -> void;
  template<typename Size, int Adjust, typename Target> auto instructionCompare(Target) -> void;
  template<typename Size, int Adjust, typename Target> auto instructionCompareRepeat(Target) -> void;
  template<typename Target, typename Source> auto instructionCompare(Target, Source) -> void;
  template<typename Target> auto instructionComplement(Target) -> void;
  auto instructionDecimalAdjustAccumulator(Register<uint8>) -> void;
  template<typename Target, typename Source> auto instructionDecrement(Target, Source) -> void;
  template<typename Target, typename Offset> auto instructionDecrementJumpNotZero(Target, Offset) -> void;
  template<typename Target, typename Source> auto instructionDivide(Target, Source) -> void;
  template<typename Target, typename Source> auto instructionDivideSigned(Target, Source) -> void;
  template<typename Target, typename Source> auto instructionExchange(Target, Source) -> void;
  template<typename Target> auto instructionExtendSign(Target) -> void;
  template<typename Target> auto instructionExtendZero(Target) -> void;
  auto instructionHalt() -> void;
  template<typename Target, typename Source> auto instructionIncrement(Target, Source) -> void;
  template<typename Source> auto instructionJump(Source) -> void;
  template<typename Source> auto instructionJumpRelative(Source) -> void;
  template<typename Target, typename Offset> auto instructionLink(Target, Offset) -> void;
  template<typename Target, typename Source> auto instructionLoad(Target, Source) -> void;
  template<typename Source, typename Offset> auto instructionLoadCarry(Source, Offset) -> void;
  template<typename Size, int Adjust> auto instructionLoad() -> void;
  template<typename Size, int Adjust> auto instructionLoadRepeat() -> void;
  template<uint Modulo, typename Target, typename Source> auto instructionModuloDecrement(Target, Source) -> void;
  template<uint Modulo, typename Target, typename Source> auto instructionModuloIncrement(Target, Source) -> void;
  auto instructionMirror(Register<uint16>) -> void;
  template<typename Target, typename Source> auto instructionMultiply(Target, Source) -> void;
  auto instructionMultiplyAdd(Register<uint16>) -> void;
  template<typename Target, typename Source> auto instructionMultiplySigned(Target, Source) -> void;
  template<typename Target> auto instructionNegate(Target) -> void;
  auto instructionNoOperation() -> void;
  template<typename Target, typename Source> auto instructionOr(Target, Source) -> void;
  template<typename Source, typename Offset> auto instructionOrCarry(Source, Offset) -> void;
  template<typename Target> auto instructionPointerAdjustAccumulator(Target) -> void;
  template<typename Target> auto instructionPop(Target) -> void;
  template<typename Source> auto instructionPush(Source) -> void;
  template<typename Target, typename Offset> auto instructionReset(Target, Offset) -> void;
  auto instructionReturn() -> void;
  template<typename Source> auto instructionReturnDeallocate(Source) -> void;
  auto instructionReturnInterrupt() -> void;
  template<typename LHS, typename RHS> auto instructionRotateLeftDigit(LHS, RHS) -> void;
  template<typename Target, typename Amount> auto instructionRotateLeft(Target, Amount) -> void;
  template<typename Target, typename Amount> auto instructionRotateLeftWithoutCarry(Target, Amount) -> void;
  template<typename LHS, typename RHS> auto instructionRotateRightDigit(LHS, RHS) -> void;
  template<typename Target, typename Amount> auto instructionRotateRight(Target, Amount) -> void;
  template<typename Target, typename Amount> auto instructionRotateRightWithoutCarry(Target, Amount) -> void;
  template<typename Target, typename Offset> auto instructionSet(Target, Offset) -> void;
  auto instructionSetCarryFlag(uint1 value) -> void;
  auto instructionSetCarryFlagComplement(uint1 value) -> void;
  template<typename Target> auto instructionSetConditionCode(uint4 code, Target) -> void;
  auto instructionSetInterruptFlipFlop(uint3 value) -> void;
  auto instructionSetRegisterFilePointer(uint2 value) -> void;
  template<typename Target, typename Amount> auto instructionShiftLeftArithmetic(Target, Amount) -> void;
  template<typename Target, typename Amount> auto instructionShiftLeftLogical(Target, Amount) -> void;
  template<typename Target, typename Amount> auto instructionShiftRightArithmetic(Target, Amount) -> void;
  template<typename Target, typename Amount> auto instructionShiftRightLogical(Target, Amount) -> void;
  template<typename Target, typename Offset> auto instructionStoreCarry(Target, Offset) -> void;
  auto instructionSoftwareInterrupt(uint3 interrupt) -> void;
  template<typename Target, typename Source> auto instructionSubtract(Target, Source) -> void;
  template<typename Target, typename Source> auto instructionSubtractBorrow(Target, Source) -> void;
  template<typename Target, typename Offset> auto instructionTestSet(Target, Offset) -> void;
  template<typename Target> auto instructionUnlink(Target) -> void;
  template<typename Target, typename Source> auto instructionXor(Target, Source) -> void;
  template<typename Source, typename Offset> auto instructionXorCarry(Source, Offset) -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

  union DataRegister {
    DataRegister() { l.l0 = 0; }
    struct { uint32 order_lsb1(l0); } l;
    struct { uint16 order_lsb2(w0, w1); } w;
    struct {  uint8 order_lsb4(b0, b1, b2, b3); } b;
  };

  struct Registers {
    DataRegister xwa[4];
    DataRegister xbc[4];
    DataRegister xde[4];
    DataRegister xhl[4];
    DataRegister xix;
    DataRegister xiy;
    DataRegister xiz;
    DataRegister xsp;
    DataRegister  pc;

    DataRegister dmas[4];
    DataRegister dmad[4];
    DataRegister dmam[4];
    DataRegister intnest;  //16-bit

    uint1 c, cp;     //carry
    uint1 n, np;     //negative
    uint1 v, vp;     //overflow or parity
    uint1 h, hp;     //half carry
    uint1 z, zp;     //zero
    uint1 s, sp;     //sign
    uint2 rfp;       //register file pointer
    uint3 iff = 7;   //interrupt mask flip-flop

    uint1 halted;   //set if halt instruction executed; waits for an interrupt to resume
    uint8 prefix;   //first opcode byte; needed for [CP|LD][ID](R) instructions
  } r;

  struct Prefetch {
    uint3  valid;  //0-4 bytes
    uint32 data;
  } p;

  //prefetch.cpp
  auto invalidate() -> void;
  auto prefetch() -> void;

  uint24 mar;  //A0-A23: memory address register
  uint16 mdr;  //D0-D15: memory data register

  static inline const Register< uint8> A{0xe0};
  static inline const Register< uint8> W{0xe1};
  static inline const Register< uint8> C{0xe4};
  static inline const Register< uint8> B{0xe5};
  static inline const Register< uint8> E{0xe8};
  static inline const Register< uint8> D{0xe9};
  static inline const Register< uint8> L{0xec};
  static inline const Register< uint8> H{0xed};

  static inline const Register<uint16> WA{0xe0};
  static inline const Register<uint16> BC{0xe4};
  static inline const Register<uint16> DE{0xe8};
  static inline const Register<uint16> HL{0xec};
  static inline const Register<uint16> IX{0xf0};
  static inline const Register<uint16> IY{0xf4};
  static inline const Register<uint16> IZ{0xf8};
  static inline const Register<uint16> SP{0xfc};

  static inline const Register<uint32> XWA{0xe0};
  static inline const Register<uint32> XBC{0xe4};
  static inline const Register<uint32> XDE{0xe8};
  static inline const Register<uint32> XHL{0xec};
  static inline const Register<uint32> XIX{0xf0};
  static inline const Register<uint32> XIY{0xf4};
  static inline const Register<uint32> XIZ{0xf8};
  static inline const Register<uint32> XSP{0xfc};

  static inline const FlagRegister F {0};
  static inline const FlagRegister FP{1};

  static inline const StatusRegister SR{};
  static inline const ProgramCounter PC{};

  static inline const ControlRegister<uint32> DMAS0{0x00};
  static inline const ControlRegister<uint32> DMAS1{0x04};
  static inline const ControlRegister<uint32> DMAS2{0x08};
  static inline const ControlRegister<uint32> DMAS3{0x0c};
  static inline const ControlRegister<uint32> DMAD0{0x10};
  static inline const ControlRegister<uint32> DMAD1{0x14};
  static inline const ControlRegister<uint32> DMAD2{0x18};
  static inline const ControlRegister<uint32> DMAD3{0x1c};
  static inline const ControlRegister<uint32> DMAM0{0x20};
  static inline const ControlRegister<uint32> DMAM1{0x24};
  static inline const ControlRegister<uint32> DMAM2{0x28};
  static inline const ControlRegister<uint32> DMAM3{0x2c};

  static inline const ControlRegister<uint16> DMAC0{0x20};
  static inline const ControlRegister<uint16> DMAC1{0x24};
  static inline const ControlRegister<uint16> DMAC2{0x28};
  static inline const ControlRegister<uint16> DMAC3{0x2c};
  static inline const ControlRegister<uint16> INTNEST{0x3c};

  static inline const uint4 False{0x00};
  static inline const uint4 True {0x08};

  static inline const uint1 Undefined = 0;

  //disassembler.cpp
#if !defined(NO_EVENTINSTRUCTION_NOTIFY)
  virtual auto disassembleRead(uint24 address) -> uint8 { return read(Byte, address); }
  noinline auto disassembleInstruction() -> string;
  noinline auto disassembleContext() -> string;
#endif
};

}
