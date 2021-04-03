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

  auto setNMI(uint1 value) -> void;
  auto setINT(uint1 value) -> void;
  auto setRES(uint1 value) -> void;
  auto setBREQ(uint1 value) -> void;

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

  auto irq(bool maskable, uint16 vector = 0x0000, uint8 extbus = 0xff) -> bool;
  auto parity(uint8) const -> bool;

  //memory.cpp
  inline auto yield() -> void {
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

  inline auto wait(uint clocks) -> void {
    yield();
    step(clocks);
  }

  inline auto opcode() -> uint8 {
    yield();
    step(1);
    return read(r.pc++);
  }

  inline auto operand() -> uint8 {
    return read(r.pc++);
  }

  inline auto operands() -> uint16 {
    uint16 data = operand() << 0;
    return data | operand() << 8;
  }

  inline auto push(uint16 x) -> void {
    write(--r.sp, x >> 8);
    write(--r.sp, x >> 0);
  }

  inline auto pop() -> uint16 {
    uint16 data = read(r.sp++) << 0;
    return data | read(r.sp++) << 8;
  }

  inline auto displace(uint16& x) -> uint16 {
    if(&x != &r.ix.word && &x != &r.iy.word) return x;
    auto d = operand();
    wait(5);
    r.wz.word = x + (int8)d;
    return r.wz.word;
  }

  //instruction.cpp
  auto instruction() -> void;
  auto instruction(uint8 code) -> void;
  auto instructionCB(uint8 code) -> void;
  auto instructionCBd(uint16 addr, uint8 code) -> void;
  auto instructionED(uint8 code) -> void;

  //algorithms.cpp
  inline auto ADD(uint8, uint8, bool = false) -> uint8;
  inline auto AND(uint8, uint8) -> uint8;
  inline auto BIT(uint3, uint8) -> uint8;
  inline auto CP (uint8, uint8) -> void;
  inline auto DEC(uint8) -> uint8;
  inline auto IN (uint8) -> uint8;
  inline auto INC(uint8) -> uint8;
  inline auto OR (uint8, uint8) -> uint8;
  inline auto RES(uint3, uint8) -> uint8;
  inline auto RL (uint8) -> uint8;
  inline auto RLC(uint8) -> uint8;
  inline auto RR (uint8) -> uint8;
  inline auto RRC(uint8) -> uint8;
  inline auto SET(uint3, uint8) -> uint8;
  inline auto SLA(uint8) -> uint8;
  inline auto SLL(uint8) -> uint8;
  inline auto SRA(uint8) -> uint8;
  inline auto SRL(uint8) -> uint8;
  inline auto SUB(uint8, uint8, bool = false) -> uint8;
  inline auto XOR(uint8, uint8) -> uint8;

  //instructions.cpp
  auto instructionADC_a_irr(uint16&) -> void;
  auto instructionADC_a_n() -> void;
  auto instructionADC_a_r(uint8&) -> void;
  auto instructionADC_hl_rr(uint16&) -> void;
  auto instructionADD_a_irr(uint16&) -> void;
  auto instructionADD_a_n() -> void;
  auto instructionADD_a_r(uint8&) -> void;
  auto instructionADD_hl_rr(uint16&) -> void;
  auto instructionAND_a_irr(uint16&) -> void;
  auto instructionAND_a_n() -> void;
  auto instructionAND_a_r(uint8&) -> void;
  auto instructionBIT_o_irr(uint3, uint16&) -> void;
  auto instructionBIT_o_irr_r(uint3, uint16&, uint8&) -> void;
  auto instructionBIT_o_r(uint3, uint8&) -> void;
  auto instructionCALL_c_nn(bool c) -> void;
  auto instructionCALL_nn() -> void;
  auto instructionCCF() -> void;
  auto instructionCP_a_irr(uint16& x) -> void;
  auto instructionCP_a_n() -> void;
  auto instructionCP_a_r(uint8& x) -> void;
  auto instructionCPD() -> void;
  auto instructionCPDR() -> void;
  auto instructionCPI() -> void;
  auto instructionCPIR() -> void;
  auto instructionCPL() -> void;
  auto instructionDAA() -> void;
  auto instructionDEC_irr(uint16&) -> void;
  auto instructionDEC_r(uint8&) -> void;
  auto instructionDEC_rr(uint16&) -> void;
  auto instructionDI() -> void;
  auto instructionDJNZ_e() -> void;
  auto instructionEI() -> void;
  auto instructionEX_irr_rr(uint16&, uint16&) -> void;
  auto instructionEX_rr_rr(uint16&, uint16&) -> void;
  auto instructionEXX() -> void;
  auto instructionHALT() -> void;
  auto instructionIM_o(uint2) -> void;
  auto instructionIN_a_in() -> void;
  auto instructionIN_r_ic(uint8&) -> void;
  auto instructionIN_ic() -> void;
  auto instructionINC_irr(uint16&) -> void;
  auto instructionINC_r(uint8&) -> void;
  auto instructionINC_rr(uint16&) -> void;
  auto instructionIND() -> void;
  auto instructionINDR() -> void;
  auto instructionINI() -> void;
  auto instructionINIR() -> void;
  auto instructionJP_c_nn(bool) -> void;
  auto instructionJP_rr(uint16&) -> void;
  auto instructionJR_c_e(bool) -> void;
  auto instructionLD_a_inn() -> void;
  auto instructionLD_a_irr(uint16& x) -> void;
  auto instructionLD_inn_a() -> void;
  auto instructionLD_inn_rr(uint16&) -> void;
  auto instructionLD_irr_a(uint16&) -> void;
  auto instructionLD_irr_n(uint16&) -> void;
  auto instructionLD_irr_r(uint16&, uint8&) -> void;
  auto instructionLD_r_n(uint8&) -> void;
  auto instructionLD_r_irr(uint8&, uint16&) -> void;
  auto instructionLD_r_r(uint8&, uint8&) -> void;
  auto instructionLD_r_r1(uint8&, uint8&) -> void;
  auto instructionLD_r_r2(uint8&, uint8&) -> void;
  auto instructionLD_rr_inn(uint16&) -> void;
  auto instructionLD_rr_nn(uint16&) -> void;
  auto instructionLD_sp_rr(uint16&) -> void;
  auto instructionLDD() -> void;
  auto instructionLDDR() -> void;
  auto instructionLDI() -> void;
  auto instructionLDIR() -> void;
  auto instructionNEG() -> void;
  auto instructionNOP() -> void;
  auto instructionOR_a_irr(uint16&) -> void;
  auto instructionOR_a_n() -> void;
  auto instructionOR_a_r(uint8&) -> void;
  auto instructionOTDR() -> void;
  auto instructionOTIR() -> void;
  auto instructionOUT_ic_r(uint8&) -> void;
  auto instructionOUT_ic() -> void;
  auto instructionOUT_in_a() -> void;
  auto instructionOUTD() -> void;
  auto instructionOUTI() -> void;
  auto instructionPOP_rr(uint16&) -> void;
  auto instructionPUSH_rr(uint16&) -> void;
  auto instructionRES_o_irr(uint3, uint16&) -> void;
  auto instructionRES_o_irr_r(uint3, uint16&, uint8&) -> void;
  auto instructionRES_o_r(uint3, uint8&) -> void;
  auto instructionRET() -> void;
  auto instructionRET_c(bool c) -> void;
  auto instructionRETI() -> void;
  auto instructionRETN() -> void;
  auto instructionRL_irr(uint16&) -> void;
  auto instructionRL_irr_r(uint16&, uint8&) -> void;
  auto instructionRL_r(uint8&) -> void;
  auto instructionRLA() -> void;
  auto instructionRLC_irr(uint16&) -> void;
  auto instructionRLC_irr_r(uint16&, uint8&) -> void;
  auto instructionRLC_r(uint8&) -> void;
  auto instructionRLCA() -> void;
  auto instructionRLD() -> void;
  auto instructionRR_irr(uint16&) -> void;
  auto instructionRR_irr_r(uint16&, uint8&) -> void;
  auto instructionRR_r(uint8&) -> void;
  auto instructionRRA() -> void;
  auto instructionRRC_irr(uint16&) -> void;
  auto instructionRRC_irr_r(uint16&, uint8&) -> void;
  auto instructionRRC_r(uint8&) -> void;
  auto instructionRRCA() -> void;
  auto instructionRRD() -> void;
  auto instructionRST_o(uint3) -> void;
  auto instructionSBC_a_irr(uint16&) -> void;
  auto instructionSBC_a_n() -> void;
  auto instructionSBC_a_r(uint8&) -> void;
  auto instructionSBC_hl_rr(uint16&) -> void;
  auto instructionSCF() -> void;
  auto instructionSET_o_irr(uint3, uint16&) -> void;
  auto instructionSET_o_irr_r(uint3, uint16&, uint8&) -> void;
  auto instructionSET_o_r(uint3, uint8&) -> void;
  auto instructionSLA_irr(uint16&) -> void;
  auto instructionSLA_irr_r(uint16&, uint8&) -> void;
  auto instructionSLA_r(uint8&) -> void;
  auto instructionSLL_irr(uint16&) -> void;
  auto instructionSLL_irr_r(uint16&, uint8&) -> void;
  auto instructionSLL_r(uint8&) -> void;
  auto instructionSRA_irr(uint16&) -> void;
  auto instructionSRA_irr_r(uint16&, uint8&) -> void;
  auto instructionSRA_r(uint8&) -> void;
  auto instructionSRL_irr(uint16&) -> void;
  auto instructionSRL_irr_r(uint16&, uint8&) -> void;
  auto instructionSRL_r(uint8&) -> void;
  auto instructionSUB_a_irr(uint16&) -> void;
  auto instructionSUB_a_n() -> void;
  auto instructionSUB_a_r(uint8&) -> void;
  auto instructionXOR_a_irr(uint16&) -> void;
  auto instructionXOR_a_n() -> void;
  auto instructionXOR_a_r(uint8&) -> void;

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
