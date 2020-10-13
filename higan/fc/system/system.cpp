#include <fc/fc.hpp>

namespace higan::Famicom {

Random random;
Scheduler scheduler;
System system;
#include "controls.cpp"
#include "serialization.cpp"

auto System::run() -> void {
  if(scheduler.enter() == Event::Frame) {
    if (!ppu.isSkipping || !ppu.skip) ppu.refresh();
  }

  auto reset = controls.reset->value();
  platform->input(controls.reset);
  if(!reset && controls.reset->value()) power(true);
}

auto System::load(Node::Object& root, Node::Object from) -> void {
  if(node) unload();

  information = {};

  node = Node::append<Node::System>(nullptr, from, interface->name());
  root = node;

  regionNode = Node::append<Node::String>(node, from, "Region", "NTSC-J → NTSC-U → PAL");
  regionNode->setAllowedValues({
    "NTSC-J → NTSC-U → PAL",
    "NTSC-U → NTSC-J → PAL",
    "PAL → NTSC-J → NTSC-U",
    "PAL → NTSC-U → NTSC-J",
    "NTSC-J",
    "NTSC-U",
    "PAL"
  });

  scheduler.reset();
  controls.load(node, from);
  cpu.load(node, from);
  apu.load(node, from);
  ppu.load(node, from);
  cartridge.load(node, from);
  controllerPort1.load(node, from);
  controllerPort2.load(node, from);
}

auto System::save() -> void {
  if(!node) return;
  cartridge.save();
}

auto System::unload() -> void {
  if(!node) return;
  save();
  cpu.unload();
  apu.unload();
  ppu.unload();
  cartridge.unload();
  controllerPort1.unload();
  controllerPort2.unload();
  node = {};
}

auto System::power(bool reset) -> void {
  for(auto& setting : node->find<Node::Setting>()) setting->setLatch();

  auto setRegion = [&](string region) {
    if(region == "NTSC-J") {
      information.region = Region::NTSCJ;
      information.frequency = Constants::Colorburst::NTSC * 6.0;
    }
    if(region == "NTSC-U") {
      information.region = Region::NTSCU;
      information.frequency = Constants::Colorburst::NTSC * 6.0;
    }
    if(region == "PAL") {
      information.region = Region::PAL;
      information.frequency = Constants::Colorburst::PAL * 6.0;
    }
  };
  auto regionsHave = regionNode->latch().split("→").strip();
  setRegion(regionsHave.first());
  for(auto& have : reverse(regionsHave)) {
    if(have == cartridge.region()) setRegion(have);
  }

  random.entropy(Random::Entropy::Low);

  cartridge.power();
  cpu.power(reset);
  apu.power(reset);
  ppu.power(reset);
  scheduler.power(cpu);

  information.serializeSize[0] = serializeInit(0);
  information.serializeSize[1] = serializeInit(1);
}

}
