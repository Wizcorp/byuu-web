#include <md/md.hpp>

namespace higan::MegaDrive {

Interface* interface = nullptr;

auto MegaDriveInterface::configure(string name, uint value) -> void {
  if(name.match("vdp/optimizeSteps")) {
    vdp.optimizeSteps = (bool) value;
  } else if(name.match("vdp/skipframe")) {
    vdp.isSkipping = (bool) value;
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
