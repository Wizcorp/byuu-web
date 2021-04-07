#include <md/md.hpp>

namespace higan::MegaDrive {

APU apu;
#include "bus.cpp"
#include "serialization.cpp"

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
  if(bus->requested() && !synchronizing()) {
    bus->grant(true);
    return step(1);
  }

  bus->grant(false);
#endif

  updateBus();

  if(!running()) {
    return step(1);
  }

  if(MegaCD() && state.nmiLine) {
    state.nmiLine = 0;  //edge-sensitive
    if(eventInterrupt->enabled()) eventInterrupt->notify("NMI");
    irq(0, 0x0066, 0xff);
  }

  if(state.intLine) {
    //level-sensitive
    if(eventInterrupt->enabled()) eventInterrupt->notify("IRQ");
    irq(1, 0x0038, 0xff);
  }

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

auto APU::updateBus() -> void {
  if(!arbstate.resetLine) return; // Z80 bus switch may be blocked by reset
  if(arbstate.busreqLine && !arbstate.busreqLatch) {
    step(9); // estimated minimum wait time to allow 68K to read back unavailable bus status (Fatal Rewind)
  }
  arbstate.busreqLatch = arbstate.busreqLine;
}

auto APU::power(bool reset) -> void {
  Z80::bus = this;
  Z80::power();
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
