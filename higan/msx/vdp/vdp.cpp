#include <msx/msx.hpp>

namespace higan::MSX {

VDP vdp;
#include "color.cpp"
#include "serialization.cpp"

auto VDP::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "VDP");
  from = Node::scan(parent = node, from);

  if(Model::MSX()) {
    screen_ = Node::append<Node::Screen>(parent, from, "Screen");
    screen_->colors(1 << 4, {&VDP::colorMSX, this});
    screen_->setSize(256, 192);
    screen_->setScale(1.0, 1.0);
    screen_->setAspect(1.0, 1.0);
    from = Node::scan(parent = screen_, from);
  }

  if(Model::MSX2()) {
    screen_ = Node::append<Node::Screen>(parent, from, "Screen");
    screen_->colors(1 << 9, {&VDP::colorMSX2, this});
    screen_->setSize(512, 424);
    screen_->setScale(0.5, 0.5);
    screen_->setAspect(1.0, 1.0);
    from = Node::scan(parent = screen_, from);
  }
}

auto VDP::unload() -> void {
  node = {};
  screen_ = {};
}

// Note: not called unless using synchro scheduler
// See: VDP::power
auto VDP::main() -> void {
  if(Model::MSX()) {
    return TMS9918::main();
  } else {
    return V9938::main();
  }
}

auto VDP::step(uint clocks) -> void {
  Thread::step(clocks);
  Thread::synchronize(cpu);
}

auto VDP::irq(bool line) -> void {
  cpu.setIRQ(line);
}

auto VDP::frame() -> void {
  scheduler.exit(Event::Frame);
}

auto VDP::refresh() -> void {
  if(Model::MSX()) {
    screen_->refresh(TMS9918::buffer, 256 * sizeof(uint32), 256, 192);
  }

  if(Model::MSX2()) {
    screen_->refresh(V9938::buffer, 512 * sizeof(uint32), 512, 424);
  }
}

auto VDP::power() -> void {
  // Note: we call directly the right main instead of calling our own
  if(Model::MSX()) {
    TMS9918::vram.allocate(16_KiB);
    TMS9918::power();
    Thread::create(system.colorburst() * 2, [&] { TMS9918::main(); });
  }

  if(Model::MSX2()) {
    V9938::videoRAM.allocate(128_KiB);
    V9938::expansionRAM.allocate(64_KiB);
    V9938::power();
    Thread::create(system.colorburst() * 2, [&] { V9938::main(); });
  }
}

/* Z80 I/O ports 0x98 - 0x9b */

auto VDP::read(uint2 port) -> uint8 {
  if(Model::MSX())
  switch(port) {
  case 0: return TMS9918::data();
  case 1: return TMS9918::status();
  }

  if(Model::MSX2())
  switch(port) {
  case 0: return V9938::data();
  case 1: return V9938::status();
  }

  return 0xff;
}

auto VDP::write(uint2 port, uint8 data) -> void {
  if(Model::MSX())
  switch(port) {
  case 0: return TMS9918::data(data);
  case 1: return TMS9918::control(data);
  }

  if(Model::MSX2())
  switch(port) {
  case 0: return V9938::data(data);
  case 1: return V9938::control(data);
  case 2: return V9938::palette(data);
  case 3: return V9938::register(data);
  }
}

}
