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

  if(!state.enabled) {
    return step(1);
  }

  if(state.nmiLine) {
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

auto APU::setNMI(bool value) -> void {
  state.nmiLine = value;
}

auto APU::setINT(bool value) -> void {
  state.intLine = value;
}

auto APU::enable(bool value) -> void {
  //68K cannot disable the Z80 without bus access
  if(!bus->granted() && !value) return;
  if(state.enabled && !value) reset();
  state.enabled = value;
}

auto APU::power(bool reset) -> void {
  Z80::bus = this;
  Z80::power();
  bus->grant(false);
  Thread::create(system.frequency() / 15.0, {&APU::main, this});
  if(!reset) ram.allocate(8_KiB);
  state = {};
}

auto APU::reset() -> void {
  Z80::power();
  bus->grant(false);
  Thread::create(system.frequency() / 15.0, {&APU::main, this});
  state = {};
}

}
