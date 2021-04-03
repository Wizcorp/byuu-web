//Motorola 68000

struct CPU : Thread {
  CPU();

  Node::Component node;
  Node::Instruction eventInstruction;
  Node::Notification eventInterrupt;

  enum class Interrupt : uint {
    Reset,
    HorizontalBlank,
    VerticalBlank,
  };


  inline auto ird() const -> uint16 { return r.ird; }

  enum : bool { User, Supervisor };
  enum : uint { Byte, Word, Long };
  enum : bool { Reverse = 1, Extend = 1, Hold = 1, Fast = 1 };

  enum : uint {
    /* 0,n */ DataRegisterDirect,
    /* 1,n */ AddressRegisterDirect,
    /* 2,n */ AddressRegisterIndirect,
    /* 3,n */ AddressRegisterIndirectWithPostIncrement,
    /* 4,n */ AddressRegisterIndirectWithPreDecrement,
    /* 5,n */ AddressRegisterIndirectWithDisplacement,
    /* 6,n */ AddressRegisterIndirectWithIndex,
    /* 7,0 */ AbsoluteShortIndirect,
    /* 7,1 */ AbsoluteLongIndirect,
    /* 7,2 */ ProgramCounterIndirectWithDisplacement,
    /* 7,3 */ ProgramCounterIndirectWithIndex,
    /* 7,4 */ Immediate,
  };

  struct Exception { enum : uint {
    Illegal,
    DivisionByZero,
    BoundsCheck,
    Overflow,
    Unprivileged,

    Trap,
    Interrupt,
  };};

  struct Vector { enum : uint {
    Reset              =  1,
    BusError           =  2,
    AddressError       =  3,
    IllegalInstruction =  4,
    DivisionByZero     =  5,
    BoundsCheck        =  6,
    Overflow           =  7,
    Unprivileged       =  8,
    Trace              =  9,
    IllegalLineA       = 10,
    IllegalLineF       = 11,
    Level1             = 25,
    Level2             = 26,
    Level3             = 27,
    Level4             = 28,
    Level5             = 29,
    Level6             = 30,
    Level7             = 31,
  };};  

  //cpu.cpp
  auto load(Node::Object, Node::Object) -> void;
  auto unload() -> void;

  auto main() -> void;

  inline auto checkForInterrupts() -> void {
    if(state.interruptPending) {
      if(state.interruptPending.bit((uint)Interrupt::HorizontalBlank)) {
        if(4 > r.i) {
          state.interruptPending.bit((uint)Interrupt::HorizontalBlank) = 0;
          //if(eventInterrupt->enabled()) eventInterrupt->notify("Hblank");
          return interrupt(Vector::Level4, 4);
        }
      }

      if(state.interruptPending.bit((uint)Interrupt::VerticalBlank)) {
        if(6 > r.i) {
          state.interruptPending.bit((uint)Interrupt::VerticalBlank) = 0;
          //if(eventInterrupt->enabled()) eventInterrupt->notify("Vblank");
          return interrupt(Vector::Level6, 6);
        }
      }
    }    
  }

  alwaysinline auto step(const uint clocks) -> void {
    refresh.ram += clocks;
    while(refresh.ram >= 133) refresh.ram -= 133;
    refresh.external += clocks;
    Thread::step(clocks);
  }

  alwaysinline auto idle(const uint clocks) -> void {
    step(clocks);
  }

  alwaysinline auto wait(const uint clocks) -> void;

  auto raise(Interrupt) -> void;
  auto lower(Interrupt) -> void;

  auto power(bool reset) -> void;

  //bus.cpp
  inline auto read(const uint1 upper, const uint1 lower, const uint24 address, uint16 data = 0) -> uint16;
  inline auto write(const uint1 upper, const uint1 lower, const uint24 address, const uint16 data) -> void;

  //io.cpp
  inline auto readIO(uint1 upper, uint1 lower, uint24 address, uint16 data) -> uint16;
  inline auto writeIO(uint1 upper, uint1 lower, uint24 address, uint16 data) -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

  auto supervisor() -> bool;
  auto exception(uint exception, uint vector, uint priority = 7) -> void;
  auto interrupt(uint vector, uint priority = 7) -> void;

  //registers.cpp
  struct DataRegister {
    explicit DataRegister(uint64 number_) : number(number_) {}
    uint3 number;
  };
  template<uint Size = Long> auto read(DataRegister reg) -> uint32;
  template<uint Size = Long> auto write(DataRegister reg, uint32 data) -> void;

  struct AddressRegister {
    explicit AddressRegister(uint64 number_) : number(number_) {}
    uint3 number;
  };
  template<uint Size = Long> auto read(AddressRegister reg) -> uint32;
  template<uint Size = Long> auto write(AddressRegister reg, uint32 data) -> void;

  auto readCCR() -> uint8;
  auto readSR() -> uint16;
  auto writeCCR(uint8 ccr) -> void;
  auto writeSR(uint16 sr) -> void;

  //memory.cpp
  template<uint Size> auto read(uint32 addr) -> uint32;
  template<uint Size, bool Order = 0> auto write(uint32 addr, uint32 data) -> void;
  template<uint Size> auto extension() -> uint32;
//
  alwaysinline auto prefetch() -> uint16 {
    wait(4);
    r.ir  = r.irc;
    r.irc = read(1, 1, r.pc & ~1);
    r.pc += 2;
    return r.ir;
  }

  //take the prefetched value without reloading the prefetch.
  //this is used by instructions such as JMP and JSR.
  alwaysinline auto prefetched() -> uint16 {
    r.ir  = r.irc;
    r.irc = 0x0000;
    r.pc += 2;
    return r.ir;
  }

  template<uint Size> auto pop() -> uint32;
  template<uint Size> auto push(uint32 data) -> void;

  //effective-address.cpp
  struct EffectiveAddress {
    explicit EffectiveAddress(uint4 mode_, uint3 reg_) : mode(mode_), reg(reg_) {
      if(mode == 7) mode += reg;  //optimization: convert modes {7; 0-4} to {7-11}
    }

    uint4 mode;
    uint3 reg;

    boolean valid;
    uint32 address;
  };

  auto prefetched(EffectiveAddress& ea) -> uint32;
  template<uint Size> auto fetch(EffectiveAddress& ea) -> uint32;
  template<uint Size, bool Hold = 0, bool Fast = 0> auto read(EffectiveAddress& ea) -> uint32;
  template<uint Size, bool Hold = 0> auto write(EffectiveAddress& ea, uint32 data) -> void;

  //instruction.cpp
  alwaysinline auto instruction() -> void {
    r.ird = r.ir;
    return instructionTable[r.ird]();
  }

  //traits.cpp
  template<uint Size> auto bytes() -> uint;
  template<uint Size> auto bits() -> uint;
  template<uint Size> auto lsb() -> uint32;
  template<uint Size> auto msb() -> uint32;
  template<uint Size> auto mask() -> uint32;
  template<uint Size> auto clip(uint32 data) -> uint32;
  template<uint Size> auto sign(uint32 data) -> int32;

  //conditions.cpp
  alwaysinline auto condition(const uint4 condition) -> const bool {
    switch(condition) {
    case  0: return true;   //T
    case  1: return false;  //F
    case  2: return !r.c && !r.z;  //HI
    case  3: return  r.c ||  r.z;  //LS
    case  4: return !r.c;  //CC,HS
    case  5: return  r.c;  //CS,LO
    case  6: return !r.z;  //NE
    case  7: return  r.z;  //EQ
    case  8: return !r.v;  //VC
    case  9: return  r.v;  //VS
    case 10: return !r.n;  //PL
    case 11: return  r.n;  //MI
    case 12: return  r.n ==  r.v;  //GE
    case 13: return  r.n !=  r.v;  //LT
    case 14: return  r.n ==  r.v && !r.z;  //GT
    case 15: return  r.n !=  r.v ||  r.z;  //LE
    }
    unreachable;
  }

  //algorithms.cpp
  template<uint Size, bool Extend = false> alwaysinline auto ADD(uint32 source, uint32 target) -> uint32;
  template<uint Size> alwaysinline auto AND(uint32 source, uint32 target) -> uint32;
  template<uint Size> alwaysinline auto ASL(uint32 result, uint shift) -> uint32;
  template<uint Size> alwaysinline auto ASR(uint32 result, uint shift) -> uint32;
  template<uint Size> alwaysinline auto CMP(uint32 source, uint32 target) -> uint32;
  template<uint Size> alwaysinline auto EOR(uint32 source, uint32 target) -> uint32;
  template<uint Size> alwaysinline auto LSL(uint32 result, uint shift) -> uint32;
  template<uint Size> alwaysinline auto LSR(uint32 result, uint shift) -> uint32;
  template<uint Size> alwaysinline auto OR(uint32 source, uint32 target) -> uint32;
  template<uint Size> alwaysinline auto ROL(uint32 result, uint shift) -> uint32;
  template<uint Size> alwaysinline auto ROR(uint32 result, uint shift) -> uint32;
  template<uint Size> alwaysinline auto ROXL(uint32 result, uint shift) -> uint32;
  template<uint Size> alwaysinline auto ROXR(uint32 result, uint shift) -> uint32;
  template<uint Size, bool Extend = false> alwaysinline auto SUB(uint32 source, uint32 target) -> uint32;

  //instructions.cpp
                      auto instructionABCD(EffectiveAddress from, EffectiveAddress with) -> void;
  template<uint Size> auto instructionADD(EffectiveAddress from, DataRegister with) -> void;
  template<uint Size> auto instructionADD(DataRegister from, EffectiveAddress with) -> void;
  template<uint Size> auto instructionADDA(EffectiveAddress from, AddressRegister with) -> void;
  template<uint Size> auto instructionADDI(EffectiveAddress with) -> void;
  template<uint Size> auto instructionADDQ(uint4 immediate, EffectiveAddress with) -> void;
  template<uint Size> auto instructionADDQ(uint4 immediate, AddressRegister with) -> void;
  template<uint Size> auto instructionADDX(EffectiveAddress with, EffectiveAddress from) -> void;
  template<uint Size> auto instructionAND(EffectiveAddress from, DataRegister with) -> void;
  template<uint Size> auto instructionAND(DataRegister from, EffectiveAddress with) -> void;
  template<uint Size> auto instructionANDI(EffectiveAddress with) -> void;
                      auto instructionANDI_TO_CCR() -> void;
                      auto instructionANDI_TO_SR() -> void;
  template<uint Size> auto instructionASL(uint4 count, DataRegister modify) -> void;
  template<uint Size> auto instructionASL(DataRegister from, DataRegister modify) -> void;
                      auto instructionASL(EffectiveAddress modify) -> void;
  template<uint Size> auto instructionASR(uint4 count, DataRegister modify) -> void;
  template<uint Size> auto instructionASR(DataRegister from, DataRegister modify) -> void;
                      auto instructionASR(EffectiveAddress modify) -> void;
                      auto instructionBCC(uint4 test, uint8 displacement) -> void;
  template<uint Size> auto instructionBCHG(DataRegister bit, EffectiveAddress with) -> void;
  template<uint Size> auto instructionBCHG(EffectiveAddress with) -> void;
  template<uint Size> auto instructionBCLR(DataRegister bit, EffectiveAddress with) -> void;
  template<uint Size> auto instructionBCLR(EffectiveAddress with) -> void;
                      auto instructionBRA(uint8 displacement) -> void;
  template<uint Size> auto instructionBSET(DataRegister bit, EffectiveAddress with) -> void;
  template<uint Size> auto instructionBSET(EffectiveAddress with) -> void;
                      auto instructionBSR(uint8 displacement) -> void;
  template<uint Size> auto instructionBTST(DataRegister bit, EffectiveAddress with) -> void;
  template<uint Size> auto instructionBTST(EffectiveAddress with) -> void;
                      auto instructionCHK(DataRegister compare, EffectiveAddress maximum) -> void;
  template<uint Size> auto instructionCLR(EffectiveAddress with) -> void;
  template<uint Size> auto instructionCMP(EffectiveAddress from, DataRegister with) -> void;
  template<uint Size> auto instructionCMPA(EffectiveAddress from, AddressRegister with) -> void;
  template<uint Size> auto instructionCMPI(EffectiveAddress with) -> void;
  template<uint Size> auto instructionCMPM(EffectiveAddress from, EffectiveAddress with) -> void;
                      auto instructionDBCC(uint4 condition, DataRegister with) -> void;
                      auto instructionDIVS(EffectiveAddress from, DataRegister with) -> void;
                      auto instructionDIVU(EffectiveAddress from, DataRegister with) -> void;
  template<uint Size> auto instructionEOR(DataRegister from, EffectiveAddress with) -> void;
  template<uint Size> auto instructionEORI(EffectiveAddress with) -> void;
                      auto instructionEORI_TO_CCR() -> void;
                      auto instructionEORI_TO_SR() -> void;
                      auto instructionEXG(DataRegister x, DataRegister y) -> void;
                      auto instructionEXG(AddressRegister x, AddressRegister y) -> void;
                      auto instructionEXG(DataRegister x, AddressRegister y) -> void;
  template<uint Size> auto instructionEXT(DataRegister with) -> void;
                      auto instructionILLEGAL(uint16 code) -> void;
                      auto instructionJMP(EffectiveAddress from) -> void;
                      auto instructionJSR(EffectiveAddress from) -> void;
                      auto instructionLEA(EffectiveAddress from, AddressRegister to) -> void;
                      auto instructionLINK(AddressRegister with) -> void;
  template<uint Size> auto instructionLSL(uint4 count, DataRegister with) -> void;
  template<uint Size> auto instructionLSL(DataRegister from, DataRegister with) -> void;
                      auto instructionLSL(EffectiveAddress with) -> void;
  template<uint Size> auto instructionLSR(uint4 count, DataRegister with) -> void;
  template<uint Size> auto instructionLSR(DataRegister from, DataRegister with) -> void;
                      auto instructionLSR(EffectiveAddress with) -> void;
  template<uint Size> auto instructionMOVE(EffectiveAddress from, EffectiveAddress to) -> void;
  template<uint Size> auto instructionMOVEA(EffectiveAddress from, AddressRegister to) -> void;
  template<uint Size> auto instructionMOVEM_TO_MEM(EffectiveAddress to) -> void;
  template<uint Size> auto instructionMOVEM_TO_REG(EffectiveAddress from) -> void;
  template<uint Size> auto instructionMOVEP(DataRegister from, EffectiveAddress to) -> void;
  template<uint Size> auto instructionMOVEP(EffectiveAddress from, DataRegister to) -> void;
                      auto instructionMOVEQ(uint8 immediate, DataRegister to) -> void;
                      auto instructionMOVE_FROM_SR(EffectiveAddress to) -> void;
                      auto instructionMOVE_TO_CCR(EffectiveAddress from) -> void;
                      auto instructionMOVE_TO_SR(EffectiveAddress from) -> void;
                      auto instructionMOVE_FROM_USP(AddressRegister to) -> void;
                      auto instructionMOVE_TO_USP(AddressRegister from) -> void;
                      auto instructionMULS(EffectiveAddress from, DataRegister with) -> void;
                      auto instructionMULU(EffectiveAddress from, DataRegister with) -> void;
                      auto instructionNBCD(EffectiveAddress with) -> void;
  template<uint Size> auto instructionNEG(EffectiveAddress with) -> void;
  template<uint Size> auto instructionNEGX(EffectiveAddress with) -> void;
                      auto instructionNOP() -> void;
  template<uint Size> auto instructionNOT(EffectiveAddress with) -> void;
  template<uint Size> auto instructionOR(EffectiveAddress from, DataRegister with) -> void;
  template<uint Size> auto instructionOR(DataRegister from, EffectiveAddress with) -> void;
  template<uint Size> auto instructionORI(EffectiveAddress with) -> void;
                      auto instructionORI_TO_CCR() -> void;
                      auto instructionORI_TO_SR() -> void;
                      auto instructionPEA(EffectiveAddress from) -> void;
                      auto instructionRESET() -> void;
  template<uint Size> auto instructionROL(uint4 count, DataRegister with) -> void;
  template<uint Size> auto instructionROL(DataRegister from, DataRegister with) -> void;
                      auto instructionROL(EffectiveAddress with) -> void;
  template<uint Size> auto instructionROR(uint4 count, DataRegister with) -> void;
  template<uint Size> auto instructionROR(DataRegister from, DataRegister with) -> void;
                      auto instructionROR(EffectiveAddress with) -> void;
  template<uint Size> auto instructionROXL(uint4 count, DataRegister with) -> void;
  template<uint Size> auto instructionROXL(DataRegister from, DataRegister with) -> void;
                      auto instructionROXL(EffectiveAddress with) -> void;
  template<uint Size> auto instructionROXR(uint4 count, DataRegister with) -> void;
  template<uint Size> auto instructionROXR(DataRegister from, DataRegister with) -> void;
                      auto instructionROXR(EffectiveAddress with) -> void;
                      auto instructionRTE() -> void;
                      auto instructionRTR() -> void;
                      auto instructionRTS() -> void;
                      auto instructionSBCD(EffectiveAddress with, EffectiveAddress from) -> void;
                      auto instructionSCC(uint4 test, EffectiveAddress to) -> void;
                      auto instructionSTOP() -> void;
  template<uint Size> auto instructionSUB(EffectiveAddress from, DataRegister with) -> void;
  template<uint Size> auto instructionSUB(DataRegister from, EffectiveAddress with) -> void;
  template<uint Size> auto instructionSUBA(EffectiveAddress from, AddressRegister with) -> void;
  template<uint Size> auto instructionSUBI(EffectiveAddress with) -> void;
  template<uint Size> auto instructionSUBQ(uint4 immediate, EffectiveAddress with) -> void;
  template<uint Size> auto instructionSUBQ(uint4 immediate, AddressRegister with) -> void;
  template<uint Size> auto instructionSUBX(EffectiveAddress from, EffectiveAddress with) -> void;
                      auto instructionSWAP(DataRegister with) -> void;
                      auto instructionTAS(EffectiveAddress with) -> void;
                      auto instructionTRAP(uint4 vector) -> void;
                      auto instructionTRAPV() -> void;
  template<uint Size> auto instructionTST(EffectiveAddress from) -> void;
                      auto instructionUNLK(AddressRegister with) -> void;

//disassembler.cpp
#if !defined(NO_EVENTINSTRUCTION_NOTIFY)
  auto disassembleInstruction(uint32 pc) -> string;
  auto disassembleContext() -> string;
#endif

  struct Registers {
    uint32 d[8];  //data registers
    uint32 a[8];  //address registers (a7 = s ? ssp : usp)
    uint32 sp;    //inactive stack pointer (s ? usp : ssp)
    uint32 pc;    //program counter

    bool c;   //carry
    bool v;   //overflow
    bool z;   //zero
    bool n;   //negative
    bool x;   //extend
    uint3 i;  //interrupt mask
    bool s;   //supervisor mode
    bool t;   //trace mode

    uint16 irc;  //instruction prefetched from external memory
    uint16 ir;   //instruction currently being decoded
    uint16 ird;  //instruction currently being executed

    bool stop;
    bool reset;
  } r;

  function<void ()> instructionTable[65536];

private:
  //disassembler.cpp
#if !defined(NO_EVENTINSTRUCTION_NOTIFY)
                      auto disassembleABCD(EffectiveAddress from, EffectiveAddress with) -> string;
  template<uint Size> auto disassembleADD(EffectiveAddress from, DataRegister with) -> string;
  template<uint Size> auto disassembleADD(DataRegister from, EffectiveAddress with) -> string;
  template<uint Size> auto disassembleADDA(EffectiveAddress from, AddressRegister with) -> string;
  template<uint Size> auto disassembleADDI(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleADDQ(uint4 immediate, EffectiveAddress with) -> string;
  template<uint Size> auto disassembleADDQ(uint4 immediate, AddressRegister with) -> string;
  template<uint Size> auto disassembleADDX(EffectiveAddress from, EffectiveAddress with) -> string;
  template<uint Size> auto disassembleAND(EffectiveAddress from, DataRegister with) -> string;
  template<uint Size> auto disassembleAND(DataRegister from, EffectiveAddress with) -> string;
  template<uint Size> auto disassembleANDI(EffectiveAddress with) -> string;
                      auto disassembleANDI_TO_CCR() -> string;
                      auto disassembleANDI_TO_SR() -> string;
  template<uint Size> auto disassembleASL(uint4 count, DataRegister with) -> string;
  template<uint Size> auto disassembleASL(DataRegister from, DataRegister with) -> string;
                      auto disassembleASL(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleASR(uint4 count, DataRegister with) -> string;
  template<uint Size> auto disassembleASR(DataRegister from, DataRegister with) -> string;
                      auto disassembleASR(EffectiveAddress with) -> string;
                      auto disassembleBCC(uint4 condition, uint8 displacement) -> string;
  template<uint Size> auto disassembleBCHG(DataRegister bit, EffectiveAddress with) -> string;
  template<uint Size> auto disassembleBCHG(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleBCLR(DataRegister bit, EffectiveAddress with) -> string;
  template<uint Size> auto disassembleBCLR(EffectiveAddress with) -> string;
                      auto disassembleBRA(uint8 displacement) -> string;
  template<uint Size> auto disassembleBSET(DataRegister bit, EffectiveAddress with) -> string;
  template<uint Size> auto disassembleBSET(EffectiveAddress with) -> string;
                      auto disassembleBSR(uint8 displacement) -> string;
  template<uint Size> auto disassembleBTST(DataRegister bit, EffectiveAddress with) -> string;
  template<uint Size> auto disassembleBTST(EffectiveAddress with) -> string;
                      auto disassembleCHK(DataRegister compare, EffectiveAddress maximum) -> string;
  template<uint Size> auto disassembleCLR(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleCMP(EffectiveAddress from, DataRegister with) -> string;
  template<uint Size> auto disassembleCMPA(EffectiveAddress from, AddressRegister with) -> string;
  template<uint Size> auto disassembleCMPI(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleCMPM(EffectiveAddress from, EffectiveAddress with) -> string;
                      auto disassembleDBCC(uint4 test, DataRegister with) -> string;
                      auto disassembleDIVS(EffectiveAddress from, DataRegister with) -> string;
                      auto disassembleDIVU(EffectiveAddress from, DataRegister with) -> string;
  template<uint Size> auto disassembleEOR(DataRegister from, EffectiveAddress with) -> string;
  template<uint Size> auto disassembleEORI(EffectiveAddress with) -> string;
                      auto disassembleEORI_TO_CCR() -> string;
                      auto disassembleEORI_TO_SR() -> string;
                      auto disassembleEXG(DataRegister x, DataRegister y) -> string;
                      auto disassembleEXG(AddressRegister x, AddressRegister y) -> string;
                      auto disassembleEXG(DataRegister x, AddressRegister y) -> string;
  template<uint Size> auto disassembleEXT(DataRegister with) -> string;
                      auto disassembleILLEGAL(uint16 code) -> string;
                      auto disassembleJMP(EffectiveAddress from) -> string;
                      auto disassembleJSR(EffectiveAddress from) -> string;
                      auto disassembleLEA(EffectiveAddress from, AddressRegister to) -> string;
                      auto disassembleLINK(AddressRegister with) -> string;
  template<uint Size> auto disassembleLSL(uint4 count, DataRegister with) -> string;
  template<uint Size> auto disassembleLSL(DataRegister from, DataRegister with) -> string;
                      auto disassembleLSL(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleLSR(uint4 count, DataRegister with) -> string;
  template<uint Size> auto disassembleLSR(DataRegister from, DataRegister with) -> string;
                      auto disassembleLSR(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleMOVE(EffectiveAddress from, EffectiveAddress to) -> string;
  template<uint Size> auto disassembleMOVEA(EffectiveAddress from, AddressRegister to) -> string;
  template<uint Size> auto disassembleMOVEM_TO_MEM(EffectiveAddress to) -> string;
  template<uint Size> auto disassembleMOVEM_TO_REG(EffectiveAddress from) -> string;
  template<uint Size> auto disassembleMOVEP(DataRegister from, EffectiveAddress to) -> string;
  template<uint Size> auto disassembleMOVEP(EffectiveAddress from, DataRegister to) -> string;
                      auto disassembleMOVEQ(uint8 immediate, DataRegister to) -> string;
                      auto disassembleMOVE_FROM_SR(EffectiveAddress to) -> string;
                      auto disassembleMOVE_TO_CCR(EffectiveAddress from) -> string;
                      auto disassembleMOVE_TO_SR(EffectiveAddress from) -> string;
                      auto disassembleMOVE_FROM_USP(AddressRegister to) -> string;
                      auto disassembleMOVE_TO_USP(AddressRegister from) -> string;
                      auto disassembleMULS(EffectiveAddress from, DataRegister with) -> string;
                      auto disassembleMULU(EffectiveAddress from, DataRegister with) -> string;
                      auto disassembleNBCD(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleNEG(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleNEGX(EffectiveAddress with) -> string;
                      auto disassembleNOP() -> string;
  template<uint Size> auto disassembleNOT(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleOR(EffectiveAddress from, DataRegister with) -> string;
  template<uint Size> auto disassembleOR(DataRegister from, EffectiveAddress with) -> string;
  template<uint Size> auto disassembleORI(EffectiveAddress with) -> string;
                      auto disassembleORI_TO_CCR() -> string;
                      auto disassembleORI_TO_SR() -> string;
                      auto disassemblePEA(EffectiveAddress from) -> string;
                      auto disassembleRESET() -> string;
  template<uint Size> auto disassembleROL(uint4 count, DataRegister with) -> string;
  template<uint Size> auto disassembleROL(DataRegister from, DataRegister with) -> string;
                      auto disassembleROL(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleROR(uint4 count, DataRegister with) -> string;
  template<uint Size> auto disassembleROR(DataRegister from, DataRegister with) -> string;
                      auto disassembleROR(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleROXL(uint4 count, DataRegister with) -> string;
  template<uint Size> auto disassembleROXL(DataRegister from, DataRegister with) -> string;
                      auto disassembleROXL(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleROXR(uint4 count, DataRegister with) -> string;
  template<uint Size> auto disassembleROXR(DataRegister from, DataRegister with) -> string;
                      auto disassembleROXR(EffectiveAddress with) -> string;
                      auto disassembleRTE() -> string;
                      auto disassembleRTR() -> string;
                      auto disassembleRTS() -> string;
                      auto disassembleSBCD(EffectiveAddress with, EffectiveAddress from) -> string;
                      auto disassembleSCC(uint4 test, EffectiveAddress to) -> string;
                      auto disassembleSTOP() -> string;
  template<uint Size> auto disassembleSUB(EffectiveAddress from, DataRegister with) -> string;
  template<uint Size> auto disassembleSUB(DataRegister from, EffectiveAddress with) -> string;
  template<uint Size> auto disassembleSUBA(EffectiveAddress from, AddressRegister with) -> string;
  template<uint Size> auto disassembleSUBI(EffectiveAddress with) -> string;
  template<uint Size> auto disassembleSUBQ(uint4 immediate, EffectiveAddress with) -> string;
  template<uint Size> auto disassembleSUBQ(uint4 immediate, AddressRegister with) -> string;
  template<uint Size> auto disassembleSUBX(EffectiveAddress from, EffectiveAddress with) -> string;
                      auto disassembleSWAP(DataRegister with) -> string;
                      auto disassembleTAS(EffectiveAddress with) -> string;
                      auto disassembleTRAP(uint4 vector) -> string;
                      auto disassembleTRAPV() -> string;
  template<uint Size> auto disassembleTST(EffectiveAddress from) -> string;
                      auto disassembleUNLK(AddressRegister with) -> string;
#endif

  template<uint Size> auto _read(uint32 addr) -> uint32;
  template<uint Size = Word> auto _readPC() -> uint32;
  auto _readDisplacement(uint32 base) -> uint32;
  auto _readIndex(uint32 base) -> uint32;
  auto _dataRegister(DataRegister dr) -> string;
  auto _addressRegister(AddressRegister ar) -> string;
  template<uint Size> auto _immediate() -> string;
  template<uint Size> auto _address(EffectiveAddress& ea) -> string;
  template<uint Size> auto _effectiveAddress(EffectiveAddress& ea) -> string;
  auto _branch(uint8 displacement) -> string;
  template<uint Size> auto _suffix() -> string;
  auto _condition(uint4 condition) -> string;

  uint32 _pc;
  function<string ()> disassembleTable[65536];

private:
  Memory::Writable<uint16> ram;
  Memory::Readable<uint16> tmss;
  uint1 tmssEnable;

  struct IO {
    boolean version;  //0 = Model 1; 1 = Model 2+
    boolean romEnable;
    boolean vdpEnable[2];
  } io;

  struct Refresh {
    uint32 ram;
     uint7 external;
  } refresh;

  struct State {
    uint32 interruptLine;
    uint32 interruptPending;
  } state;
};

extern CPU cpu;
