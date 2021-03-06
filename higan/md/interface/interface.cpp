#include <md/md.hpp>

namespace higan::MegaDrive {

Interface* interface = nullptr;

auto MegaDriveInterface::configure(string name, double value) -> void {
  if(name.match("vdp/skipframe")) {
    vdp.isSkipping = (bool) value;
  } else if(name.match("cpu/overclock")) {
    cpu.overclock = value / 7.0;
    cpu.setFrequency(system.frequency() * cpu.overclock);
  } else if(name.match("apu/overclock")) {
    apu.overclock = value / 15.0;
    apu.setFrequency(system.frequency() * apu.overclock);
  } else if(name.match("vdp/overclock")) {
    vdp.overclock = value / 2.0;
    vdp.setFrequency(system.frequency() * vdp.overclock);
  }
}

auto MegaDriveInterface::game() -> string {
  if(expansion.node && (!cartridge.node || !cartridge.bootable())) {
    if(mcd.disc) return mcd.name();
    return expansion.name();
  }

  if(cartridge.node && cartridge.bootable()) {
    return cartridge.name();
  }

  return "(no cartridge connected)";
}

auto MegaDriveInterface::root() -> Node::Object {
  return system.node;
}

auto MegaDriveInterface::load(Node::Object& root, string tree) -> void {
  interface = this;
  system.load(root, Node::unserialize(tree));
}

auto MegaDriveInterface::unload() -> void {
  system.unload();
}

auto MegaDriveInterface::save() -> void {
  system.save();
}

auto MegaDriveInterface::power() -> void {
  system.power(false);
}

auto MegaDriveInterface::run() -> void {
  system.run();
}

auto MegaDriveInterface::serialize(bool synchronize) -> serializer {
  return system.serialize(synchronize);
}

auto MegaDriveInterface::unserialize(serializer& s) -> bool {
  return system.unserialize(s);
}

}
