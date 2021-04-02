#include <md/md.hpp>

namespace higan::MegaDrive {

CPU cpu;
#include "bus.cpp"
#include "io.cpp"
#include "serialization.cpp"

enum : uint { Byte, Word, Long };
enum : bool { Reverse = 1 };

#include "registers.cpp"
#include "memory.cpp"
#include "effective-address.cpp"
#include "traits.cpp"
#include "conditions.cpp"
#include "algorithms.cpp"
#include "instructions.cpp"

#if !defined(NO_EVENTINSTRUCTION_NOTIFY)
#include "disassembler.cpp"
#endif

#include "instruction.cpp"

auto CPU::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "CPU");
  from = Node::scan(parent = node, from);

  eventInstruction = Node::append<Node::Instruction>(parent, from, "Instruction", "CPU");
  eventInstruction->setAddressBits(24);

  eventInterrupt = Node::append<Node::Notification>(parent, from, "Interrupt", "CPU");
}

auto CPU::unload() -> void {
  eventInstruction = {};
  eventInterrupt = {};
  node = {};
}


auto CPU::supervisor() -> bool {
  if(r.s) return true;

  r.pc -= 4;
  exception(Exception::Unprivileged, Vector::Unprivileged);
  return false;
}

auto CPU::exception(uint exception, uint vector, uint priority) -> void {
  idle(10);  //todo: not accurate

  auto pc = r.pc;
  auto sr = readSR();

  if(exception != Exception::Illegal) {
    if(!r.s) swap(r.a[7], r.sp);
    r.i = priority;
    r.s = 1;
    r.t = 0;
  }

  push<Long>(pc - 4);
  push<Word>(sr);

  r.pc = read<Long>(vector << 2);
  prefetch();
  prefetch();
}

auto CPU::interrupt(uint vector, uint priority) -> void {
  return exception(Exception::Interrupt, vector, priority);
}


auto CPU::main() -> void {
#if defined(SCHEDULER_SYNCHRO)
  if (vdp.hasRendered) {
    vdp.hasRendered = false;
    event = Event::Frame;
    return;
  }
#endif

  if(state.interruptPending) {
    if(state.interruptPending.bit((uint)Interrupt::Reset)) {
      state.interruptPending.bit((uint)Interrupt::Reset) = 0;
      r.a[7] = read(1, 1, 0) << 16 | read(1, 1, 2) << 0;
      r.pc   = read(1, 1, 4) << 16 | read(1, 1, 6) << 0;
      prefetch();
      prefetch();
      if(eventInterrupt->enabled()) eventInterrupt->notify("Reset");
    }

    if(state.interruptPending.bit((uint)Interrupt::HorizontalBlank)) {
      if(4 > r.i) {
        state.interruptPending.bit((uint)Interrupt::HorizontalBlank) = 0;
        if(eventInterrupt->enabled()) eventInterrupt->notify("Hblank");
        return interrupt(Vector::Level4, 4);
      }
    }

    if(state.interruptPending.bit((uint)Interrupt::VerticalBlank)) {
      if(6 > r.i) {
        state.interruptPending.bit((uint)Interrupt::VerticalBlank) = 0;
        if(eventInterrupt->enabled()) eventInterrupt->notify("Vblank");
        return interrupt(Vector::Level6, 6);
      }
    }
  }

  #if !defined(NO_EVENTINSTRUCTION_NOTIFY)
  if(eventInstruction->enabled() && eventInstruction->address(r.pc - 4)) {
    eventInstruction->notify(disassembleInstruction(r.pc - 4), disassembleContext());
  }
  #endif
  
  instruction();
}

auto CPU::step(uint clocks) -> void {
  refresh.ram += clocks;
  while(refresh.ram >= 133) refresh.ram -= 133;
  refresh.external += clocks;
  Thread::step(clocks);
}

auto CPU::idle(uint clocks) -> void {
  step(clocks);
}

auto CPU::wait(uint clocks) -> void {  
  while(vdp.dma.active) {
    Thread::step(1);
    Thread::synchronize(vdp);
  }

  step(clocks);
  Thread::synchronize();
}

auto CPU::raise(Interrupt interrupt) -> void {
  if(!state.interruptLine.bit((uint)interrupt)) {
    state.interruptLine.bit((uint)interrupt) = 1;
    state.interruptPending.bit((uint)interrupt) = 1;
  }
}

auto CPU::lower(Interrupt interrupt) -> void {
  state.interruptLine.bit((uint)interrupt) = 0;
  state.interruptPending.bit((uint)interrupt) = 0;
}

auto CPU::power(bool reset) -> void {
  for(auto& dr : r.d) dr = 0;
  for(auto& ar : r.a) ar = 0;
  r.sp = 0;
  r.pc = 0;

  r.c = 0;
  r.v = 0;
  r.z = 0;
  r.n = 0;
  r.x = 0;
  r.i = 7;
  r.s = 1;
  r.t = 0;

  r.irc = 0x4e71;  //nop
  r.ir  = 0x4e71;  //nop
  r.ird = 0x4e71;  //nop

  r.stop  = false;
  r.reset = false;

  Thread::create(system.frequency() / 7.0, {&CPU::main, this});

  ram.allocate(64_KiB >> 1);

  tmssEnable = false;
  if(system.tmss->value()) {
    tmss.allocate(2_KiB >> 1);
    if(auto fp = platform->open(system.node, "tmss.rom", File::Read, File::Required)) {
      for(uint address : range(tmss.size())) tmss.program(address, fp->readm(2));
      tmssEnable = true;
    }
  }

  if(!reset) memory::fill(ram.data(), sizeof(ram));

  io = {};
  io.version = tmssEnable;
  io.romEnable = !tmssEnable;
  io.vdpEnable[0] = !tmssEnable;
  io.vdpEnable[1] = !tmssEnable;

  refresh = {};

  state = {};
  state.interruptPending.bit((uint)Interrupt::Reset) = 1;
}

}
