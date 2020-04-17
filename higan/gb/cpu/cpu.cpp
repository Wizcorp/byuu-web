#include <gb/gb.hpp>

namespace higan::GameBoy {

#define SP r.sp.word
#define PC r.pc.word

#include "io.cpp"
#include "memory.cpp"
#include "timing.cpp"
#include "serialization.cpp"
CPU cpu;

auto CPU::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "CPU");
  from = Node::scan(parent = node, from);

  if(Model::GameBoy()) {
    version = Node::append<Node::String>(parent, from, "Version", "DMG-CPU B");
    version->setAllowedValues({
      "DMG-CPU",
      "DMG-CPU A",
      "DMG-CPU B",
      "DMG-CPU C",
      "CPU MGB",
    });
  }

  if(Model::SuperGameBoy()) {
    version = Node::append<Node::String>(parent, from, "Version", "SGB-CPU 01");
    version->setAllowedValues({
      "SGB-CPU 01",
      "CPU SGB2"
    });
  }

  if(Model::GameBoyColor()) {
    version = Node::append<Node::String>(parent, from, "Version", "CPU CGB");
    version->setAllowedValues({
      "CPU CGB",
      "CPU CGB A",
      "CPU CGB B",
      "CPU CGB C",
      "CPU CGB D",
      "CPU CGB E",
    });
  }

  string origin = Model::SuperGameBoy() ? "SGB" : "CPU";

  eventInstruction = Node::append<Node::Instruction>(parent, from, "Instruction", origin);
  eventInstruction->setAddressBits(16);

  eventInterrupt = Node::append<Node::Notification>(parent, from, "Interrupt", origin);
}

auto CPU::unload() -> void {
  node = {};
  version = {};
  eventInstruction = {};
  eventInterrupt = {};
}

auto CPU::main() -> void {
  if(status.hblankPending) {
    status.hblankPending = 0;
    hblankTrigger();
  }

  //are interrupts enabled?
  if(r.ime) {
    //are any interrupts pending?
    if(status.interruptLatch) {
      if(eventInterrupt->enabled()) eventInterrupt->notify("IRQ");

      idle();
      idle();
      idle();
      r.ime = 0;
      write(--SP, PC >> 8);  //upper byte may write to IE before it is polled again
      uint8 mask = status.interruptFlag & status.interruptEnable;
      write(--SP, PC >> 0);  //lower byte write to IE has no effect
      if(mask) {
        uint interruptID = bit::first(mask);  //find highest priority interrupt
        lower(interruptID);
        PC = 0x0040 + interruptID * 8;
      } else {
        //if push(PCH) writes to IE and disables all requested interrupts, PC is forced to zero
        PC = 0x0000;
      }
    }
  }

  #if !defined(NO_EVENTINSTRUCTION_NOTIFY)
  if(eventInstruction->enabled() && eventInstruction->address(PC)) {
    eventInstruction->notify(disassembleInstruction(), disassembleContext());
  }
  #endif
  
  instruction();

  if(Model::SuperGameBoy()) {
    scheduler.exit(Event::Step);
  }
}

auto CPU::raised(uint interruptID) const -> bool {
  return status.interruptFlag.bit(interruptID);
}

auto CPU::raise(uint interruptID) -> void {
  status.interruptFlag.bit(interruptID) = 1;
  if(status.interruptEnable.bit(interruptID)) {
    r.halt = false;
    if(interruptID == Interrupt::Joypad) r.stop = false;
  }
}

auto CPU::lower(uint interruptID) -> void {
  status.interruptFlag.bit(interruptID) = 0;
}

auto CPU::stoppable() -> bool {
  if(status.speedSwitch) {
    status.speedSwitch = 0;
    status.speedDouble ^= 1;
    if(status.speedDouble == 0) setFrequency(4 * 1024 * 1024);
    if(status.speedDouble == 1) setFrequency(8 * 1024 * 1024);
    return false;
  }
  return true;
}

auto CPU::power() -> void {
  Thread::create(4 * 1024 * 1024, {&CPU::main, this});
  SM83::power();

  for(auto& n : wram) n = 0x00;
  for(auto& n : hram) n = 0x00;

  status = {};

  //note: this may not be accurate, DIV values at startup may be due to LCD delays?
  //but probably not, since "CPU CGB" differs from "CPU CGB [A-E]"

  //0146~0149
  if(version->latch() == "DMG-CPU"   ) status.div = 0x0146;

  //ffe6~ffe9
  if(version->latch() == "DMG-CPU A" ) status.div = 0xffe6;
  if(version->latch() == "DMG-CPU B" ) status.div = 0xffe6;
  if(version->latch() == "DMG-CPU C" ) status.div = 0xffe6;
  if(version->latch() == "CPU MGB"   ) status.div = 0xffe6;

  if(version->latch() == "SGB-CPU 01") status.div = 0xffe6;  //unconfirmed
  if(version->latch() == "CPU SGB2"  ) status.div = 0xffe6;  //unconfirmed

  //0206~0209
  if(version->latch() == "CPU CGB"   ) status.div = 0x0206;

  //fffa~fffd
  if(version->latch() == "CPU CGB A" ) status.div = 0xfffa;
  if(version->latch() == "CPU CGB B" ) status.div = 0xfffa;
  if(version->latch() == "CPU CGB C" ) status.div = 0xfffa;
  if(version->latch() == "CPU CGB D" ) status.div = 0xfffa;
  if(version->latch() == "CPU CGB E" ) status.div = 0xfffa;
}

}
