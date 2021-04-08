// #if defined(PROFILE_PERFORMANCE) && !defined(EMSCRIPTEN)
#if defined(PROFILE_PERFORMANCE)
#include "../vdp-performance/vdp.cpp"
#else
#include <md/md.hpp>

namespace higan::MegaDrive {

VDP vdp;
#include "memory.cpp"
#include "io.cpp"
#include "dma.cpp"
#include "render.cpp"
#include "background.cpp"
#include "sprite.cpp"
#include "color.cpp"
#include "serialization.cpp"

auto VDP::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "VDP");
  from = Node::scan(parent = node, from);

  screen = Node::append<Node::Screen>(parent, from, "Screen");
  screen->colors(3 * (1 << 9), {&VDP::color, this});
  screen->setSize(1280, 480);
  screen->setScale(0.25, 0.50);
  screen->setAspect(1.0, 1.0);
  from = Node::scan(parent = screen, from);

  region = Node::append<Node::String>(parent, from, "Region", "PAL", [&](auto region) {
    if(region == "NTSC") screen->setSize(1280, 448);
    if(region == "PAL" ) screen->setSize(1280, 480);
  });
  region->setDynamic(true);
  region->setAllowedValues({"NTSC", "PAL"});
}

auto VDP::unload() -> void {
  node = {};
  screen = {};
  region = {};
}

static uint counter = 1;
static uint dmaCounter = 0;

auto VDP::main() -> void {
#if defined(SCHEDULER_SYNCHRO)
  if (dmaCounter > 0) {
    dma.run();
    Thread::step(1);
    Thread::synchronize(apu);
    dmaCounter--;
    return;
  }
#endif

  switch(counter) {
    case 1:
      scanline();

      cpu.lower(CPU::Interrupt::HorizontalBlank);
      apu.setINT(false);

      if(state.vcounter == 0) {
        latch.horizontalInterruptCounter = io.horizontalInterruptCounter;
        io.vblankIRQ = false;
        cpu.lower(CPU::Interrupt::VerticalBlank);
      }

      if(state.vcounter < screenHeight()) {
        counter = 2;
      } else if(state.vcounter == screenHeight()) {
        if(io.verticalBlankInterruptEnable) {
          io.vblankIRQ = true;
          cpu.raise(CPU::Interrupt::VerticalBlank);
        }

        // only stay high for ~2573/2 clocks
        apu.setINT(true);
        counter = 4;
        return step(2573/2);
      } else {
        counter = 5;
        return step(1710);
      }
      break;
    case 2:
      if(state.hcounter < 1280) {
        run();
        state.hdot++;
        return step(pixelWidth());
      }

      counter = 3;
      break;

    case 3:
      if(latch.horizontalInterruptCounter-- == 0) {
        latch.horizontalInterruptCounter = io.horizontalInterruptCounter;
        if(io.horizontalBlankInterruptEnable) {
          cpu.raise(CPU::Interrupt::HorizontalBlank);
        }
      }

      counter = 5;
      return step(430);
      break;
      
    case 4:
      apu.setINT(false);
      counter = 5;
      return step(1710-(2573/2));
      break;

    case 5:
      state.hdot = 0;
      state.hcounter = 0;
      if(++state.vcounter >= frameHeight()) {
        state.vcounter = 0;
        state.field ^= 1;
        latch.overscan = io.overscan;
      }
      latch.displayWidth = io.displayWidth;
      counter = 1;
      break;
  }  
}

auto VDP::step(uint clocks) -> void {
  state.hcounter += clocks;

#if defined(SCHEDULER_SYNCHRO)
  dmaCounter = clocks - 1;
  dma.run();
  Thread::step(1);
  Thread::synchronize(apu);
#else
  while(clocks--) {
    dma.run();
    Thread::step(1);
    Thread::synchronize(cpu, apu);
  }
#endif
}

auto VDP::refresh() -> void {
  auto data = output;

  if(region->value() == "NTSC") {
    if(latch.overscan) data += 16 * 1280;
    screen->refresh(data, 1280 * sizeof(uint32), 1280, 448);
  }

  if(region->value() == "PAL") {
    if(!latch.overscan) data -= 16 * 1280;
    screen->refresh(data, 1280 * sizeof(uint32), 1280, 480);
  }
}

auto VDP::power(bool reset) -> void {
  Thread::create(system.frequency() * overclock, {&VDP::main, this});

  output = buffer + 16 * 1280;  //overscan offset

  if(!reset) {
    for(auto& data : vram.memory) data = 0;
    for(auto& data : vsram.memory) data = 0;
    for(auto& data : cram.memory) data = 0;
  }

  vram.mode = 0;
  io = {};
  latch = {};
  state = {};

  planeA.power();
  window.power();
  planeB.power();
  sprite.power();
  dma.power();

  psg.power(reset);
}

}
#endif
