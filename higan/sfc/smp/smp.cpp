#include <sfc/sfc.hpp>

namespace higan::SuperFamicom {

SMP smp;
#include "memory.cpp"
#include "io.cpp"
#include "timing.cpp"
#include "serialization.cpp"

auto SMP::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "SMP");
  from = Node::scan(parent = node, from);

  eventInstruction = Node::append<Node::Instruction>(parent, from, "Instruction", "SMP");
  eventInstruction->setAddressBits(16);
}

auto SMP::unload() -> void {
  eventInstruction = {};
  node = {};
}

auto SMP::main() -> void {
  #if defined(SCHEDULER_SYNCHRO)
  if (!firstRun) {
    Thread::synchronize(dsp); 
  }
  #endif

  firstRun = 0;

  if(r.wait) return instructionWait();
  if(r.stop) return instructionStop();

  if(eventInstruction->enabled() && eventInstruction->address(r.pc.w)) {
    eventInstruction->notify(disassembleInstruction(), disassembleContext());
  }
  instruction();
}

auto SMP::power(bool reset) -> void {
  if(auto fp = platform->open(system.node, "ipl.rom", File::Read, File::Required)) {
    fp->read(iplrom, 64);
  }

  SPC700::power();
  create(system.apuFrequency() / 12.0, {&SMP::main, this});

  r.pc.byte.l = iplrom[62];
  r.pc.byte.h = iplrom[63];

  io = {};
  timer0 = {};
  timer1 = {};
  timer2 = {};
}

}
