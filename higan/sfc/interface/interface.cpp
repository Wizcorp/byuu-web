#include <sfc/sfc.hpp>

namespace higan::SuperFamicom {

Interface* interface = nullptr;

auto SuperFamicomInterface::configure(string name, double value) -> void {
  if(name.match("cpu/lockstep")) {
    cpu.lockstep.enabled = (bool) value;
  } else if(name.match("cpu/fastmath")) {
    cpu.fastMath = (bool) value;
  } else if(name.match("cpu/overclock")) {
    cpu.overclock = value;
  } else if(name.match("smp/lockstep")) {
    smp.lockstep.enabled = (bool) value;
  } else if(name.match("dsp/enabled")) {
    dsp.enabled = (bool) value;
  } else if(name.match("ppu/skipframe")) {
    ppu.isSkipping = (bool) value;
  }
}

auto SuperFamicomInterface::game() -> string {
  #if defined(CORE_GB)
  if(cartridge.has.GameBoySlot && GameBoy::cartridge.node) {
    return GameBoy::cartridge.name();
  }
  #endif

  if(bsmemory.node) {
    return {cartridge.name(), " + ", bsmemory.name()};
  }

  if(sufamiturboA.node && sufamiturboB.node) {
    return {sufamiturboA.name(), " + ", sufamiturboB.name()};
  }

  if(sufamiturboA.node) {
    return sufamiturboA.name();
  }

  if(sufamiturboB.node) {
    return sufamiturboB.name();
  }

  if(cartridge.node) {
    return cartridge.name();
  }

  return "(no cartridge connected)";
}

auto SuperFamicomInterface::root() -> Node::Object {
  return system.node;
}

auto SuperFamicomInterface::load(Node::Object& root, string tree) -> void {
  interface = this;
  system.load(root, Node::unserialize(tree));
}

auto SuperFamicomInterface::unload() -> void {
  system.unload();
}

auto SuperFamicomInterface::save() -> void {
  system.save();
}

auto SuperFamicomInterface::power() -> void {
  system.power(false);
}

auto SuperFamicomInterface::run() -> void {
  system.run();
}

auto SuperFamicomInterface::serialize(bool synchronize) -> serializer {
  return system.serialize(synchronize);
}

auto SuperFamicomInterface::unserialize(serializer& s) -> bool {
  return system.unserialize(s);
}

auto SuperFamicomInterface::exportMemory() -> bool {
  directory::create("/tmpfs/Memory/Super Famicom/");
  file::write("/tmpfs/Memory/Super Famicom/wram.bin", {cpu.wram, 128 * 1024});
  file::write("/tmpfs/Memory/Super Famicom/vram.bin", {ppu.vram.data, 64 * 1024});
  return true;
}

}
