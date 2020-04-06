#include <ngp/interface/interface.hpp>

struct NeoGeoPocket : Emulator {
  NeoGeoPocket();
  auto load() -> bool override;
  auto open(higan::Node::Object, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> override;
  auto input(higan::Node::Input) -> void override;
};

struct NeoGeoPocketColor : Emulator {
  NeoGeoPocketColor();
  auto load() -> bool override;
  auto open(higan::Node::Object, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> override;
  auto input(higan::Node::Input) -> void override;
};

NeoGeoPocket::NeoGeoPocket() {
  interface = new higan::NeoGeoPocket::NeoGeoPocketInterface;
  name = "Neo Geo Pocket";
  extensions = {"ngp"};

  firmware.append({"BIOS", "World", "0293555b21c4fac516d25199df7809b26beeae150e1d4504a050db32264a6ad7"});
}

auto NeoGeoPocket::load() -> bool {
  if(!file::exists(firmware[0].location)) {
    errorFirmwareRequired(firmware[0]);
    return false;
  }

  if(auto port = root->find<higan::Node::Port>("Cartridge Slot")) {
    auto peripheral = port->allocate();
    port->connect(peripheral);
  }

  if(auto node = root->find<higan::Node::Boolean>("Fast Boot")) {
    node->setValue(true);
  }

  return true;
}

auto NeoGeoPocket::open(higan::Node::Object node, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> {
  if(name == "manifest.bml") return Emulator::manifest();

  if(name == "bios.rom") {
    return loadFirmware(firmware[0]);
  }

  auto document = BML::unserialize(game.manifest);
  auto programROMSize = document["game/board/memory(content=Program,type=Flash)/size"].natural();

  if(name == "program.flash") {
    auto location = locate(game.location, ".sav", settings.paths.saves);
    if(auto result = vfs::fs::file::open(location, mode)) return result;
    if(mode == vfs::file::mode::read) return vfs::memory::file::open(game.image.data(), programROMSize);
  }

  return {};
}

auto NeoGeoPocket::input(higan::Node::Input node) -> void {
  auto name = node->name();
  maybe<InputMapping&> mapping;
  if(name == "Up"    ) mapping = virtualPad.up;
  if(name == "Down"  ) mapping = virtualPad.down;
  if(name == "Left"  ) mapping = virtualPad.left;
  if(name == "Right" ) mapping = virtualPad.right;
  if(name == "A"     ) mapping = virtualPad.a;
  if(name == "B"     ) mapping = virtualPad.b;
  if(name == "Option") mapping = virtualPad.start;

  if(mapping) {
    auto value = mapping->value();
    if(auto button = node->cast<higan::Node::Button>()) {
      button->setValue(value);
    }
  }
}

NeoGeoPocketColor::NeoGeoPocketColor() {
  interface = new higan::NeoGeoPocket::NeoGeoPocketColorInterface;
  name = "Neo Geo Pocket Color";
  extensions = {"ngpc", "ngc"};

  firmware.append({"BIOS", "World", "8fb845a2f71514cec20728e2f0fecfade69444f8d50898b92c2259f1ba63e10d"});
}

auto NeoGeoPocketColor::load() -> bool {
  if(!file::exists(firmware[0].location)) {
    errorFirmwareRequired(firmware[0]);
    return false;
  }

  if(auto port = root->find<higan::Node::Port>("Cartridge Slot")) {
    auto peripheral = port->allocate();
    port->connect(peripheral);
  }

  if(auto node = root->find<higan::Node::Boolean>("Fast Boot")) {
    node->setValue(true);
  }

  return true;
}

auto NeoGeoPocketColor::open(higan::Node::Object node, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> {
  if(name == "manifest.bml") return Emulator::manifest();

  if(name == "bios.rom") {
    return loadFirmware(firmware[0]);
  }

  auto document = BML::unserialize(game.manifest);
  auto programROMSize = document["game/board/memory(content=Program,type=Flash)/size"].natural();

  if(name == "program.flash") {
    auto location = locate(game.location, ".sav", settings.paths.saves);
    if(auto result = vfs::fs::file::open(location, mode)) return result;
    if(mode == vfs::file::mode::read) return vfs::memory::file::open(game.image.data(), programROMSize);
  }

  return {};
}

auto NeoGeoPocketColor::input(higan::Node::Input node) -> void {
  auto name = node->name();
  maybe<InputMapping&> mapping;
  if(name == "Up"    ) mapping = virtualPad.up;
  if(name == "Down"  ) mapping = virtualPad.down;
  if(name == "Left"  ) mapping = virtualPad.left;
  if(name == "Right" ) mapping = virtualPad.right;
  if(name == "A"     ) mapping = virtualPad.a;
  if(name == "B"     ) mapping = virtualPad.b;
  if(name == "Option") mapping = virtualPad.start;

  if(mapping) {
    auto value = mapping->value();
    if(auto button = node->cast<higan::Node::Button>()) {
      button->setValue(value);
    }
  }
}
