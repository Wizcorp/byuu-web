//Zilog Z80

struct APU : Thread {
  Node::Component node;
  Node::Instruction eventInstruction;
  Node::Notification eventInterrupt;

  constexpr inline auto synchronizing() const -> bool { return scheduler.synchronizing(); }
  constexpr inline auto requested() -> bool { return _requested; }
  constexpr inline auto granted() -> bool { return _granted; }

  constexpr inline auto request(bool value) -> void { _requested = value; }
  constexpr inline auto grant(bool value) -> void { _granted = value; }

  //z80.cpp
  auto load(Node::Object, Node::Object) -> void;
  auto unload() -> void;

  auto main() -> void;
  inline auto step(uint clocks) -> void;

  auto power(bool reset) -> void;

  constexpr inline auto running() -> bool { return arbstate.resetLine && busStatus(); }

  alwaysinline auto setNMI(uint1 value) -> void {
    state.nmiLine = value;
  }

  alwaysinline auto setINT(uint1 value) -> void {
    state.intLine = value;
    if (state.intLine) {
      state.interruptPending = true;
    }
  }

  alwaysinline auto setRES(uint1 value) -> void {
    if(!value && arbstate.resetLine) {
      power(true);
    }
    arbstate.resetLine = value;
  }

  alwaysinline auto setBREQ(uint1 value) -> void {
    arbstate.busreqLine = value;
  }

  auto updateBus() -> void;

  inline auto busStatus() -> uint1 {
    // 0->68K, 1->Z80
    return (arbstate.resetLine & arbstate.busreqLatch) ^ 1;
  } 

  //bus.cpp
  inline auto read(const uint16 address) -> uint8;
  inline auto write(const uint16 address, const uint8 data) -> void;

  inline auto in(const uint16 address) -> uint8;
  inline auto out(const uint16 address, const uint8 data) -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;


  //CMOS: out (c) writes 0x00
  //NMOS: out (c) writes 0xff;
  //      if an interrupt fires during "ld a,i" or "ld a,r", PF is cleared
  enum class MOSFET : uint { CMOS, NMOS };

  alwaysinline auto irq() -> bool;

  constexpr auto parity(uint8_t value) const -> bool {
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return !(value & 1);
  }

  //memory.cpp
  alwaysinline auto yield() -> void {
    // freeze Z80, allow external access until relinquished
    // when using synchro, move this logic to the component's main loop
    // (generally found in the inheriting component, like the Mega Drive APU) 
  #if !defined(SCHEDULER_SYNCHRO)
    if(requested()) {
      grant(true);
      while(requested() && !synchronizing()) step(1);
      grant(false);
    }
  #endif
  }

  alwaysinline auto wait(uint clocks) -> void {
    yield();
    step(clocks);
  }

  alwaysinline auto opcode() -> uint8 {
    yield();
    step(1);
    return read(r.pc++);
  }

  alwaysinline auto operand() -> uint8 {
    return read(r.pc++);
  }

  alwaysinline auto operands() -> uint16 {
    uint16 data = operand() << 0;
    return data | operand() << 8;
  }

  alwaysinline auto push(uint16 x) -> void {
    write(--r.sp, x >> 8);
    write(--r.sp, x >> 0);
  }

  alwaysinline auto pop() -> uint16 {
    uint16 data = read(r.sp++) << 0;
    return data | read(r.sp++) << 8;
  }

  alwaysinline auto displace(uint16& x) -> uint16 {
    if(&x != &r.ix.word && &x != &r.iy.word) return x;
    auto d = operand();
    wait(5);
    r.wz.word = x + (int8)d;
    return r.wz.word;
  }

  //instruction.cpp
  alwaysinline auto instruction() -> void;
  alwaysinline auto instruction(uint8 code) -> void;
  alwaysinline auto instructionCB(uint8 code) -> void;
  alwaysinline auto instructionCBd(uint16 addr, uint8 code) -> void;
  alwaysinline auto instructionED(uint8 code) -> void;

  //algorithms.cpp
  alwaysinline auto ADD(uint8, uint8, bool = false) -> uint8;
  alwaysinline auto AND(uint8, uint8) -> uint8;
  alwaysinline auto BIT(uint3, uint8) -> uint8;
  alwaysinline auto CP (uint8, uint8) -> void;
  alwaysinline auto DEC(uint8) -> uint8;
  alwaysinline auto IN (uint8) -> uint8;
  alwaysinline auto INC(uint8) -> uint8;
  alwaysinline auto OR (uint8, uint8) -> uint8;
  alwaysinline auto RES(uint3, uint8) -> uint8;
  alwaysinline auto RL (uint8) -> uint8;
  alwaysinline auto RLC(uint8) -> uint8;
  alwaysinline auto RR (uint8) -> uint8;
  alwaysinline auto RRC(uint8) -> uint8;
  alwaysinline auto SET(uint3, uint8) -> uint8;
  alwaysinline auto SLA(uint8) -> uint8;
  alwaysinline auto SLL(uint8) -> uint8;
  alwaysinline auto SRA(uint8) -> uint8;
  alwaysinline auto SRL(uint8) -> uint8;
  alwaysinline auto SUB(uint8, uint8, bool = false) -> uint8;
  alwaysinline auto XOR(uint8, uint8) -> uint8;

  //instructions.cpp
  alwaysinline auto instructionADC_a_irr(uint16&) -> void;
  alwaysinline auto instructionADC_a_n() -> void;
  alwaysinline auto instructionADC_a_r(uint8&) -> void;
  alwaysinline auto instructionADC_hl_rr(uint16&) -> void;
  alwaysinline auto instructionADD_a_irr(uint16&) -> void;
  alwaysinline auto instructionADD_a_n() -> void;
  alwaysinline auto instructionADD_a_r(uint8&) -> void;
  alwaysinline auto instructionADD_hl_rr(uint16&) -> void;
  alwaysinline auto instructionAND_a_irr(uint16&) -> void;
  alwaysinline auto instructionAND_a_n() -> void;
  alwaysinline auto instructionAND_a_r(uint8&) -> void;
  alwaysinline auto instructionBIT_o_irr(uint3, uint16&) -> void;
  alwaysinline auto instructionBIT_o_irr_r(uint3, uint16&, uint8&) -> void;
  alwaysinline auto instructionBIT_o_r(uint3, uint8&) -> void;
  alwaysinline auto instructionCALL_c_nn(bool c) -> void;
  alwaysinline auto instructionCALL_nn() -> void;
  alwaysinline auto instructionCCF() -> void;
  alwaysinline auto instructionCP_a_irr(uint16& x) -> void;
  alwaysinline auto instructionCP_a_n() -> void;
  alwaysinline auto instructionCP_a_r(uint8& x) -> void;
  alwaysinline auto instructionCPD() -> void;
  alwaysinline auto instructionCPDR() -> void;
  alwaysinline auto instructionCPI() -> void;
  alwaysinline auto instructionCPIR() -> void;
  alwaysinline auto instructionCPL() -> void;
  alwaysinline auto instructionDAA() -> void;
  alwaysinline auto instructionDEC_irr(uint16&) -> void;
  alwaysinline auto instructionDEC_r(uint8&) -> void;
  alwaysinline auto instructionDEC_rr(uint16&) -> void;
  alwaysinline auto instructionDI() -> void;
  alwaysinline auto instructionDJNZ_e() -> void;
  alwaysinline auto instructionEI() -> void;
  alwaysinline auto instructionEX_irr_rr(uint16&, uint16&) -> void;
  alwaysinline auto instructionEX_rr_rr(uint16&, uint16&) -> void;
  alwaysinline auto instructionEXX() -> void;
  alwaysinline auto instructionHALT() -> void;
  alwaysinline auto instructionIM_o(uint2) -> void;
  alwaysinline auto instructionIN_a_in() -> void;
  alwaysinline auto instructionIN_r_ic(uint8&) -> void;
  alwaysinline auto instructionIN_ic() -> void;
  alwaysinline auto instructionINC_irr(uint16&) -> void;
  alwaysinline auto instructionINC_r(uint8&) -> void;
  alwaysinline auto instructionINC_rr(uint16&) -> void;
  alwaysinline auto instructionIND() -> void;
  alwaysinline auto instructionINDR() -> void;
  alwaysinline auto instructionINI() -> void;
  alwaysinline auto instructionINIR() -> void;
  alwaysinline auto instructionJP_c_nn(bool) -> void;
  alwaysinline auto instructionJP_rr(uint16&) -> void;
  alwaysinline auto instructionJR_c_e(bool) -> void;
  alwaysinline auto instructionLD_a_inn() -> void;
  alwaysinline auto instructionLD_a_irr(uint16& x) -> void;
  alwaysinline auto instructionLD_inn_a() -> void;
  alwaysinline auto instructionLD_inn_rr(uint16&) -> void;
  alwaysinline auto instructionLD_irr_a(uint16&) -> void;
  alwaysinline auto instructionLD_irr_n(uint16&) -> void;
  alwaysinline auto instructionLD_irr_r(uint16&, uint8&) -> void;
  alwaysinline auto instructionLD_r_n(uint8&) -> void;
  alwaysinline auto instructionLD_r_irr(uint8&, uint16&) -> void;
  alwaysinline auto instructionLD_r_r(uint8&, uint8&) -> void;
  alwaysinline auto instructionLD_r_r1(uint8&, uint8&) -> void;
  alwaysinline auto instructionLD_r_r2(uint8&, uint8&) -> void;
  alwaysinline auto instructionLD_rr_inn(uint16&) -> void;
  alwaysinline auto instructionLD_rr_nn(uint16&) -> void;
  alwaysinline auto instructionLD_sp_rr(uint16&) -> void;
  alwaysinline auto instructionLDD() -> void;
  alwaysinline auto instructionLDDR() -> void;
  alwaysinline auto instructionLDI() -> void;
  alwaysinline auto instructionLDIR() -> void;
  alwaysinline auto instructionNEG() -> void;
  alwaysinline auto instructionNOP() -> void;
  alwaysinline auto instructionOR_a_irr(uint16&) -> void;
  alwaysinline auto instructionOR_a_n() -> void;
  alwaysinline auto instructionOR_a_r(uint8&) -> void;
  alwaysinline auto instructionOTDR() -> void;
  alwaysinline auto instructionOTIR() -> void;
  alwaysinline auto instructionOUT_ic_r(uint8&) -> void;
  alwaysinline auto instructionOUT_ic() -> void;
  alwaysinline auto instructionOUT_in_a() -> void;
  alwaysinline auto instructionOUTD() -> void;
  alwaysinline auto instructionOUTI() -> void;
  alwaysinline auto instructionPOP_rr(uint16&) -> void;
  alwaysinline auto instructionPUSH_rr(uint16&) -> void;
  alwaysinline auto instructionRES_o_irr(uint3, uint16&) -> void;
  alwaysinline auto instructionRES_o_irr_r(uint3, uint16&, uint8&) -> void;
  alwaysinline auto instructionRES_o_r(uint3, uint8&) -> void;
  alwaysinline auto instructionRET() -> void;
  alwaysinline auto instructionRET_c(bool c) -> void;
  alwaysinline auto instructionRETI() -> void;
  alwaysinline auto instructionRETN() -> void;
  alwaysinline auto instructionRL_irr(uint16&) -> void;
  alwaysinline auto instructionRL_irr_r(uint16&, uint8&) -> void;
  alwaysinline auto instructionRL_r(uint8&) -> void;
  alwaysinline auto instructionRLA() -> void;
  alwaysinline auto instructionRLC_irr(uint16&) -> void;
  alwaysinline auto instructionRLC_irr_r(uint16&, uint8&) -> void;
  alwaysinline auto instructionRLC_r(uint8&) -> void;
  alwaysinline auto instructionRLCA() -> void;
  alwaysinline auto instructionRLD() -> void;
  alwaysinline auto instructionRR_irr(uint16&) -> void;
  alwaysinline auto instructionRR_irr_r(uint16&, uint8&) -> void;
  alwaysinline auto instructionRR_r(uint8&) -> void;
  alwaysinline auto instructionRRA() -> void;
  alwaysinline auto instructionRRC_irr(uint16&) -> void;
  alwaysinline auto instructionRRC_irr_r(uint16&, uint8&) -> void;
  alwaysinline auto instructionRRC_r(uint8&) -> void;
  alwaysinline auto instructionRRCA() -> void;
  alwaysinline auto instructionRRD() -> void;
  alwaysinline auto instructionRST_o(uint3) -> void;
  alwaysinline auto instructionSBC_a_irr(uint16&) -> void;
  alwaysinline auto instructionSBC_a_n() -> void;
  alwaysinline auto instructionSBC_a_r(uint8&) -> void;
  alwaysinline auto instructionSBC_hl_rr(uint16&) -> void;
  alwaysinline auto instructionSCF() -> void;
  alwaysinline auto instructionSET_o_irr(uint3, uint16&) -> void;
  alwaysinline auto instructionSET_o_irr_r(uint3, uint16&, uint8&) -> void;
  alwaysinline auto instructionSET_o_r(uint3, uint8&) -> void;
  alwaysinline auto instructionSLA_irr(uint16&) -> void;
  alwaysinline auto instructionSLA_irr_r(uint16&, uint8&) -> void;
  alwaysinline auto instructionSLA_r(uint8&) -> void;
  alwaysinline auto instructionSLL_irr(uint16&) -> void;
  alwaysinline auto instructionSLL_irr_r(uint16&, uint8&) -> void;
  alwaysinline auto instructionSLL_r(uint8&) -> void;
  alwaysinline auto instructionSRA_irr(uint16&) -> void;
  alwaysinline auto instructionSRA_irr_r(uint16&, uint8&) -> void;
  alwaysinline auto instructionSRA_r(uint8&) -> void;
  alwaysinline auto instructionSRL_irr(uint16&) -> void;
  alwaysinline auto instructionSRL_irr_r(uint16&, uint8&) -> void;
  alwaysinline auto instructionSRL_r(uint8&) -> void;
  alwaysinline auto instructionSUB_a_irr(uint16&) -> void;
  alwaysinline auto instructionSUB_a_n() -> void;
  alwaysinline auto instructionSUB_a_r(uint8&) -> void;
  alwaysinline auto instructionXOR_a_irr(uint16&) -> void;
  alwaysinline auto instructionXOR_a_n() -> void;
  alwaysinline auto instructionXOR_a_r(uint8&) -> void;

  //disassembler.cpp
#if !defined(NO_EVENTINSTRUCTION_NOTIFY)
  auto disassembleInstruction(maybe<uint16> pc = {}) -> string;
  auto disassembleContext() -> string;

  auto disassemble(uint16 pc, uint8 prefix, uint8 code) -> string;
  auto disassembleCB(uint16 pc, uint8 prefix, uint8 code) -> string;
  auto disassembleCBd(uint16 pc, uint8 prefix, int8 d, uint8 code) -> string;
  auto disassembleED(uint16 pc, uint8 prefix, uint8 code) -> string;
#endif

  MOSFET mosfet = MOSFET::NMOS;
  enum class Prefix : uint { hl, ix, iy } prefix = Prefix::hl;

  struct Registers {
    union Pair {
      Pair() : word(0) {}
      uint16 word;
      struct Byte { uint8 order_msb2(hi, lo); } byte;
    };

    Pair af, af_;
    Pair bc, bc_;
    Pair de, de_;
    Pair hl, hl_;
    Pair ix;
    Pair iy;
    Pair ir;
    Pair wz;
    uint16 sp;
    uint16 pc;

    boolean ei;    //"ei" executed last
    boolean p;     //"ld a,i" or "ld a,r" executed last
    boolean q;     //opcode that updated flag registers executed last
    boolean halt;  //HALT instruction executed
    boolean iff1;  //interrupt flip-flop 1
    boolean iff2;  //interrupt flip-flop 2
    uint2   im;    //interrupt mode (0-2)
  } r;

private:
  Memory::Writable<uint8> ram;

  struct IO {
    uint9 bank;
  } io;

  struct ArbState {
    uint1 resetLine;
    uint1 busreqLine = 1;
    uint1 busreqLatch = 1;
  } arbstate;

  struct State {
    uint1 nmiLine;
    uint1 intLine;
    uint1 interruptPending;
  } state;

  bool _requested;
  bool _granted;
};

extern APU apu;
