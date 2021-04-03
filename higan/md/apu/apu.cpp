#include <md/md.hpp>

namespace higan::MegaDrive {

APU apu;
#include "bus.cpp"
#include "serialization.cpp"

#if !defined(NO_EVENTINSTRUCTION_NOTIFY)
#include "disassembler.cpp"
#endif

#include "registers.cpp"
#include "instruction.cpp"
#include "algorithms.cpp"
#include "instructions.cpp"

auto APU::irq(bool maskable, uint16 pc, uint8 extbus) -> bool {
  if((maskable && !IFF1) || EI) return false;
  uint cycles;
  R.bit(0,6)++;

  push(PC);

  switch(maskable ? IM : (uint2)1) {

  case 0: {
    //external data bus ($ff = RST $38)
    WZ = extbus;
    cycles = extbus|0x38 == 0xFF ? 6 : 7;
    break;
  }

  case 1: {
    //constant address
    WZ = pc;
    cycles = maskable ? 7 : 5;
    break;
  }

  case 2: {
    //vector table with external data bus
    uint16 addr = I << 8 | extbus;
    WZL = read(addr + 0);
    WZH = read(addr + 1);
    cycles = 7;
    break;
  }

  }

  PC = WZ;
  IFF1 = 0;
  if(maskable) IFF2 = 0;
  HALT = 0;
  if(P) PF = 0;
  P = 0;
  Q = 0;

  wait(cycles);
  return true;
}

auto APU::parity(uint8 value) const -> bool {
  value ^= value >> 4;
  value ^= value >> 2;
  value ^= value >> 1;
  return !(value & 1);
}

auto APU::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "APU");
  from = Node::scan(parent = node, from);

  eventInstruction = Node::append<Node::Instruction>(parent, from, "Instruction", "APU");
  eventInstruction->setAddressBits(16);

  eventInterrupt = Node::append<Node::Notification>(parent, from, "Interrupt", "APU");
}

auto APU::unload() -> void {
  eventInstruction = {};
  eventInterrupt = {};
  node = {};
}

auto APU::main() -> void {  
// See: higan/component/processor/z80/memory.cpp
#if defined(SCHEDULER_SYNCHRO)
  if(requested() && !synchronizing()) {
    grant(true);
    return step(1);
  }

  grant(false);
#endif

  updateBus();

  if(!running()) {
    return step(1);
  }
 
 /* MD doesn't use NMI, so we can skip the check!
  if(state.nmiLine) {
    state.nmiLine = 0;  //edge-sensitive
    if(eventInterrupt->enabled()) eventInterrupt->notify("NMI");
    irq(0, 0x0066, 0xff);
  }*/

  #if !defined(NO_EVENTINSTRUCTION_NOTIFY)
  if(eventInstruction->enabled() && eventInstruction->address(r.pc)) {
    eventInstruction->notify(disassembleInstruction(), disassembleContext());
  }
  #endif
  
  instruction();
}

auto APU::step(uint clocks) -> void {
  Thread::step(clocks);
#if defined(SCHEDULER_SYNCHRO)
  Thread::synchronize(psg, ym2612);
#else
  Thread::synchronize(cpu, vdp, psg, ym2612);
#endif
}

auto APU::setNMI(uint1 value) -> void {
  state.nmiLine = value;
}

auto APU::setINT(uint1 value) -> void {
  state.intLine = value;
  if (state.intLine) {
    state.interruptPending = true;
  }
}

auto APU::setRES(uint1 value) -> void {
  if(!value && arbstate.resetLine) {
    power(true);
  }
  arbstate.resetLine = value;
}

auto APU::setBREQ(uint1 value) -> void {
  arbstate.busreqLine = value;
}

auto APU::updateBus() -> void {
  if(!arbstate.resetLine) return; // Z80 bus switch may be blocked by reset
  if(arbstate.busreqLine && !arbstate.busreqLatch) {
    step(9); // estimated minimum wait time to allow 68K to read back unavailable bus status (Fatal Rewind)
  }
  arbstate.busreqLatch = arbstate.busreqLine;
}

auto APU::power(bool reset) -> void {
  mosfet = MOSFET::NMOS;

  prefix = Prefix::hl;
  r = {};
  AF = 0xffff;
  SP = 0xffff;
  IFF1 = 0;
  IFF2 = 0;
  IM = 1;

  ym2612.power(reset);
  Thread::create(system.frequency() / 15.0, {&APU::main, this});
  if(!reset) {
    ram.allocate(8_KiB);
    arbstate = {};
  }
  state = {};
  io = {};
}

}
