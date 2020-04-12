#include <sg/sg.hpp>

namespace higan::SG1000 {

VDP vdp;
#include "color.cpp"
#include "serialization.cpp"

auto VDP::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "VDP");
  from = Node::scan(parent = node, from);

  screen = Node::append<Node::Screen>(parent, from, "Screen");
  screen->colors(1 << 4, {&VDP::color, this});
  screen->setSize(256, 192);
  screen->setScale(1.0, 1.0);
  screen->setAspect(1.0, 1.0);
  node = Node::scan(parent = screen, from);
}

auto VDP::unload() -> void {
  node = {};
  screen = {};
}

auto VDP::main() -> void {
  return TMS9918::main();
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
  screen->refresh(buffer, 256 * sizeof(uint32), 256, 192);
}

auto VDP::power() -> void {
  TMS9918::power();
  Thread::create(system.colorburst() * 2, [&] { main(); });
  vram.allocate(0x4000);
}

}
