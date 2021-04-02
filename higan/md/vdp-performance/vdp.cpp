#include <md/md.hpp>

namespace higan::MegaDrive {

VDP vdp;
#include "memory.cpp"

#include "dma.cpp"
#include "background.cpp"
#include "object.cpp"
#include "sprite.cpp"
#include "color.cpp"
#include "serialization.cpp"

auto VDP::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "VDP");
  from = Node::scan(parent = node, from);

  screen = Node::append<Node::Screen>(parent, from, "Screen");
  screen->colors(3 * (1 << 9), {&VDP::color, this});
  screen->setSize(640, 480);
  screen->setScale(0.50, 0.50);
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

static uint counter = 0;
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

  switch(counter++) {
    case 1:
      //H = 0
      cpu.lower(CPU::Interrupt::HorizontalBlank);
      apu.setINT(false);

      if(state.vcounter == 0) {
        latch.displayWidth = io.displayWidth;
        latch.horizontalInterruptCounter = io.horizontalInterruptCounter;
        io.vblankIRQ = false;
        cpu.lower(CPU::Interrupt::VerticalBlank);
      }

      if(state.vcounter == screenHeight()) {
        if(io.verticalBlankInterruptEnable) {
          io.vblankIRQ = true;
          cpu.raise(CPU::Interrupt::VerticalBlank);
        }
        apu.setINT(true);
      }

      step(512);
      break;
    case 2:
      //H = 512
      if(state.vcounter < screenHeight() && !runAhead() && (!isSkipping || !skip)) {
        render();
      }

      step(768);
      break;
    case 3:
      //H = 1280
      if(state.vcounter < screenHeight()) {
        if(latch.horizontalInterruptCounter-- == 0) {
          latch.horizontalInterruptCounter = io.horizontalInterruptCounter;
          if(io.horizontalBlankInterruptEnable) {
            cpu.raise(CPU::Interrupt::HorizontalBlank);
          }
        }
      }
      step(430);
      break;
    case 4:
      //H = 0
      state.hdot = 0;
      state.hcounter = 0;
      state.vcounter++;

      if(state.vcounter == 240) {
#if defined(SCHEDULER_SYNCHRO)
        if (!isSkipping || !skip) {
          refresh();
        }

        skip = !skip;
        hasRendered = true;
#else
        scheduler.exit(Event::Frame);
#endif
      }

      if(state.vcounter >= frameHeight()) {
        state.vcounter = 0;
        state.field ^= 1;
        latch.interlace = io.interlaceMode == 3;
        latch.overscan = io.overscan;
      }
      counter = 0;
      break;
  }
}

auto VDP::step(uint clocks) -> void {
  state.hcounter += clocks;

  if(optimizeSteps && (!dma.io.enable || dma.io.wait)) {
    dma.active = 0;
    Thread::step(clocks);
#if defined(SCHEDULER_SYNCHRO)
    Thread::synchronize(apu);
#else
    Thread::synchronize(cpu, apu);
#endif
  } else {
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
}

auto VDP::refresh() -> void {
  auto data = output;

  if(region->value() == "NTSC") {
    if(latch.overscan) data += (8 << latch.interlace) * 320;
    screen->refresh(data, 320 * sizeof(uint32), screenWidth(), 224 << latch.interlace);
  }

  if(region->value() == "PAL") {
    if(!latch.overscan) data -= (8 << latch.interlace) * 320;
    screen->refresh(data, 320 * sizeof(uint32), screenWidth(), 240 << latch.interlace);
  }
}

auto VDP::render() -> void {
  auto output = this->output;
  const uint y = state.vcounter;
  if(!latch.interlace) {
    output += y * 320;
  } else {
    output += (y * 2 + state.field) * 320;
  }
  if(!io.displayEnable) return (void)memory::fill<uint32>(output, screenWidth());

  if(y < window.verticalOffsetXorDirection) {
    window.renderWindow(0, screenWidth());
  } else if(!window.io.horizontalDirection) {
    window.renderWindow(0, window.io.horizontalOffset);
    planeA.renderScreen(window.io.horizontalOffset, screenWidth());
  } else {
    planeA.renderScreen(0, window.io.horizontalOffset);
    window.renderWindow(window.io.horizontalOffset, screenWidth());
  }
  planeB.renderScreen(0, screenWidth());
  sprite.render();

  auto A = &planeA.pixels[0];
  auto B = &planeB.pixels[0];
  auto S = &sprite.pixels[0];
  uint7 c[4] = {0, 0, 0, io.backgroundColor};
  if(!io.shadowHighlightEnable) {
    auto p = &cram.palette[1 << 7];
    for(uint x : range(screenWidth())) {
      c[0] = *A++;
      c[1] = *B++;
      c[2] = *S++;
      uint l = lookupFG[c[0] >> 2 << 10 | c[1] >> 2 << 5 | c[2] >> 2];
      *output++ = p[c[l]];
    }
  } else {
    auto p = &cram.palette[0 << 7];
    for(uint x : range(screenWidth())) {
      c[0] = *A++;
      c[1] = *B++;
      c[2] = *S++;
      uint l = lookupFG[c[0] >> 2 << 10 | c[1] >> 2 << 5 | c[2] >> 2];
      uint mode = (c[0] | c[1]) >> 2 & 1;  //0 = shadow, 1 = normal, 2 = highlight
      if(l == 2) {
        if(c[2] >= 0x70) {
          if(c[2] <= 0x72) mode = 1;
          else if(c[2] == 0x73) l = lookupBG[c[0] >> 2 << 5 | c[1] >> 2], mode++;
          else if(c[2] == 0x7b) l = lookupBG[c[0] >> 2 << 5 | c[1] >> 2], mode = 0;
          else mode |= c[2] >> 2 & 1;
        } else {
          mode |= c[2] >> 2 & 1;
        }
      }
      *output++ = p[mode << 7 | c[l]];
    }
  }
}

auto VDP::power(bool reset) -> void {
  Thread::create(system.frequency() / 2.0, {&VDP::main, this});

  for(auto& pixel : buffer) pixel = 0;
  output = buffer + 16 * 320;  //overscan offset

  for(auto& data : vram.pixels) data = 0;
  for(auto& data : vram.memory) data = 0;
  vram.size = 32768;
  vram.mode = 0;

  for(auto& data : vsram.memory) data = 0;

  for(auto& data : cram.memory) data = 0;
  for(auto& data : cram.palette) data = 0;

  dma.power();
  psg.power(reset);

  planeA.io = {};
  window.io = {};
  planeB.io = {};

  sprite.io = {};
  for(auto& object : sprite.oam) object = {};
  for(auto& object : sprite.objects) object = {};

  state = {};
  io = {};
  latch = {};

  static bool initialized = false;
  if(!initialized) {
    initialized = true;

    for(uint a = 0; a < 32; a++) {
      for(uint b = 0; b < 32; b++) {
        uint ap = a & 1, ac = a >> 1;
        uint bp = b & 1, bc = b >> 1;
        uint bg = (ap && ac) || ac && !(bp && bc) ? 0 : bc ? 1 : 3;
        lookupBG[a << 5 | b] = bg;
      }
    }

    for(uint a = 0; a < 32; a++) {
      for(uint b = 0; b < 32; b++) {
        for(uint s = 0; s < 32; s++) {
          uint ap = a & 1, ac = a >> 1;
          uint bp = b & 1, bc = b >> 1;
          uint sp = s & 1, sc = s >> 1;
          uint bg = (ap && ac) || ac && !(bp && bc) ? 0 : bc ? 1 : 3;
          uint fg = (sp && sc) || sc && !(bp && bc) && !(ap && ac) ? 2 : bg;
          lookupFG[a << 10 | b << 5 | s] = fg;
        }
      }
    }
  }
}

}
