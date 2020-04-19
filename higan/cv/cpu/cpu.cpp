#include <cv/cv.hpp>

namespace higan::ColecoVision {

CPU cpu;
#include "memory.cpp"
#include "serialization.cpp"

auto CPU::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "CPU");
  from = Node::scan(parent = node, from);

  eventInstruction = Node::append<Node::Instruction>(parent, from, "Instruction", "CPU");
  eventInstruction->setAddressBits(16);

  eventInterrupt = Node::append<Node::Notification>(parent, from, "Interrupt", "CPU");
}

auto CPU::unload() -> void {
  eventInstruction = {};
  eventInterrupt = {};
  node = {};
}

auto CPU::main() -> void {
  if(state.nmiLine) {
    state.nmiLine = 0;  //edge-sensitive
    if(eventInterrupt->enabled()) eventInterrupt->notify("NMI");
    irq(0, 0x0066, 0xff);
  }

  if(state.irqLine) {
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

auto CPU::step(uint clocks) -> void {
  Thread::step(clocks);
  Thread::synchronize();
}

auto CPU::setNMI(bool value) -> void {
  state.nmiLine = value;
}

auto CPU::setIRQ(bool value) -> void {
  state.irqLine = value;
}

auto CPU::power() -> void {
  Z80::bus = this;
  Z80::power();
  Thread::create(system.colorburst(), {&CPU::main, this});

  if(Model::ColecoVision()) ram.allocate(0x0400), expansion.allocate(0x1000);
  if(Model::ColecoAdam())   ram.allocate(0x0400), expansion.allocate(0x1000);

  r.pc = 0x0000;  //reset vector address
  state = {};
  io = {};
}

}
