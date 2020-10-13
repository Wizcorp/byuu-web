#include <fc/fc.hpp>

namespace higan::Famicom {

CPU cpu;
#include "memory.cpp"
#include "timing.cpp"
#include "serialization.cpp"

auto CPU::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "CPU");
  from = Node::scan(parent = node, from);

  eventInstruction = Node::append<Node::Instruction>(parent, from, "Instruction", "CPU");
  eventInstruction->setAddressBits(16);
//eventInstruction->setEnabled(true);

  eventInterrupt = Node::append<Node::Notification>(parent, from, "Interrupt", "CPU");
//eventInterrupt->setEnabled(true);

  syncOnce = Node::append<Node::Boolean>(parent, from, "Sync once", true);
  syncOnce->setDynamic(true);
  syncOnce->setValue(true);
}

auto CPU::unload() -> void {
  eventInstruction = {};
  eventInterrupt = {};
  node = {};
  syncOnce = {};
}

auto CPU::main() -> void {
  if(io.interruptPending) {
    if(eventInterrupt->enabled()) eventInterrupt->notify("IRQ");
    return interrupt();
  }

  #if !defined(NO_EVENTINSTRUCTION_NOTIFY)
  if(eventInstruction->enabled() && eventInstruction->address(r.pc)) {
    eventInstruction->notify(disassembleInstruction(), disassembleContext());
  }
  #endif
  
  instruction();

  if(syncOnce->value() == 1) {
    Thread::synchronize();
  }
}

auto CPU::step(uint clocks) -> void {
  Thread::step(clocks);
  if(syncOnce->value() == 0) {
    Thread::synchronize();
  }
}

auto CPU::power(bool reset) -> void {
  MOS6502::BCD = 0;
  MOS6502::power();
  Thread::create(system.frequency(), {&CPU::main, this});

  if(!reset) for(auto& data : ram) data = 0xff;
  ram[0x008] = 0xf7;  //todo: what is this about?
  ram[0x009] = 0xef;
  ram[0x00a] = 0xdf;
  ram[0x00f] = 0xbf;

  r.pc.byte(0) = bus.read(0xfffc);
  r.pc.byte(1) = bus.read(0xfffd);

  io = {};
}

}
