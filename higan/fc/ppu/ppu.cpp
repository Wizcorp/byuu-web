#include <fc/fc.hpp>

namespace higan::Famicom {

PPU ppu;
#include "memory.cpp"
#include "render.cpp"
#include "color.cpp"
#include "serialization.cpp"

auto PPU::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "PPU");
  from = Node::scan(parent = node, from);

  screen = Node::append<Node::Screen>(parent, from, "Screen");
  screen->colors(1 << 9, {&PPU::color, this});
  screen->setSize(256, 240);
  screen->setScale(1.0, 1.0);
  screen->setAspect(8.0, 7.0);
  from = Node::scan(parent = screen, from);

  overscan = Node::append<Node::Boolean>(parent, from, "Overscan", false, [&](auto overscan) {
    if(overscan) screen->setSize(256, 240);
    else screen->setSize(256, 224);
  });
  overscan->setDynamic(true);

  colorEmulation = Node::append<Node::Boolean>(parent, from, "Color Emulation", true, [&](auto value) {
    screen->resetPalette();
  });
  colorEmulation->setDynamic(true);
}

auto PPU::unload() -> void {
  node = {};
  screen = {};
  overscan = {};
  colorEmulation = {};
}

auto PPU::main() -> void {
  renderScanline();

  #if defined(SCHEDULER_SYNCHRO)
  Thread::step(rate() * 341);
  Thread::synchronize(cpu);
  #endif
}

auto PPU::step(uint clocks) -> void {
  uint L = vlines();

  while(clocks--) {
    #if defined(SCHEDULER_SYNCHRO)
    if(io.ly == 241 && io.lx ==   0) io.nmiFlag = io.nmiHold = 1;
    if(io.ly == 241 && io.lx ==   2) cpu.nmiLine(io.nmiEnable && io.nmiFlag);

    if(io.ly == L-2 && io.lx == 340) io.spriteZeroHit = 0, io.spriteOverflow = 0;

    if(io.ly == L-1 && io.lx ==   0) io.nmiFlag = io.nmiHold = 0;
    if(io.ly == L-1 && io.lx ==   2) cpu.nmiLine(io.nmiEnable && io.nmiFlag);
    #else
    if(io.ly == 240 && io.lx == 340) io.nmiHold = 1;
    if(io.ly == 241 && io.lx ==   0) io.nmiFlag = io.nmiHold;
    if(io.ly == 241 && io.lx ==   2) cpu.nmiLine(io.nmiEnable && io.nmiFlag);

    if(io.ly == L-2 && io.lx == 340) io.spriteZeroHit = 0, io.spriteOverflow = 0;

    if(io.ly == L-2 && io.lx == 340) io.nmiHold = 0;
    if(io.ly == L-1 && io.lx ==   0) io.nmiFlag = io.nmiHold;
    if(io.ly == L-1 && io.lx ==   2) cpu.nmiLine(io.nmiEnable && io.nmiFlag);

    Thread::step(rate());
    Thread::synchronize(cpu);
    #endif

    io.lx++;
  }
}

auto PPU::scanline() -> void {
  io.lx = 0;
  if(++io.ly == vlines()) {
    io.ly = 0;
    frame();
  }
  cartridge.scanline(io.ly);
}

auto PPU::frame() -> void {
  io.field++;
  scheduler.exit(Event::Frame);
}

auto PPU::refresh() -> void {
  if(overscan->value() == 0) {
    screen->refresh(buffer + 8 * 256, 256 * sizeof(uint32), 256, 224);
  }

  if(overscan->value() == 1) {
    screen->refresh(buffer, 256 * sizeof(uint32), 256, 240);
  }
}

auto PPU::power(bool reset) -> void {
  Thread::create(system.frequency(), {&PPU::main, this});

  io = {};
  latch = {};

  if(!reset) {
    for(auto& data : ciram ) data = 0;
    for(auto& data : cgram ) data = 0;
    for(auto& data : oam   ) data = 0;
  }

  for(auto& data : buffer) data = 0;
}

}
