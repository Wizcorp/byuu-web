#include <gb/interface/interface.hpp>

struct GameBoy : Emulator {
  GameBoy();
  auto load() -> bool override;
  auto open(higan::Node::Object, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> override;
};

struct GameBoyColor : Emulator {
  GameBoyColor();
  auto load() -> bool override;
  auto open(higan::Node::Object, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> override;
};

GameBoy::GameBoy() {
  interface = new higan::GameBoy::GameBoyInterface;
  name = "Game Boy";
  extensions = {"gb"};
  ports = {};
  buttons = {
      "Up", "Down", "Left", "Right", 
      "B", "A"
      "Select", "Start"
  };
}

auto GameBoy::load() -> bool {
  if(auto port = root->find<higan::Node::Port>("Cartridge Slot")) {
    auto peripheral = port->allocate();
    port->connect(peripheral);
  }

  return true;
}

auto GameBoy::open(higan::Node::Object node, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> {
  if(name == "manifest.bml") return Emulator::manifest();

  if(name == "boot.dmg-1.rom") {
    return vfs::memory::file::open(Resource::GameBoy::BootDMG1, sizeof Resource::GameBoy::BootDMG1);
  }

  auto document = BML::unserialize(game.manifest);
  auto programROMSize = document["game/board/memory(content=Program,type=ROM)/size"].natural();
  auto saveRAMVolatile = (bool)document["game/board/memory(Content=Save,type=RAM)/volatile"];

  if(name == "program.rom") {
    return vfs::memory::file::open(game.image.data(), programROMSize);
  }

  if(name == "save.ram" && !saveRAMVolatile) {
    auto location = locate(game.location, ".sav", Emulator::GameFolder);
    if(auto result = vfs::fs::file::open(location, mode)) return result;
  }

  return {};
}

GameBoyColor::GameBoyColor() {
  interface = new higan::GameBoy::GameBoyColorInterface;
  name = "Game Boy Color";
  extensions = {"gbc"};
  ports = {
    "Controller"
  };
  buttons = {
      "Up", "Down", "Left", "Right", 
      "B", "A"
      "Select", "Start"
  };
}

auto GameBoyColor::load() -> bool {
  if(auto port = root->find<higan::Node::Port>("Cartridge Slot")) {
    auto peripheral = port->allocate();
    port->connect(peripheral);
  }

  return true;
}

auto GameBoyColor::open(higan::Node::Object node, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> {
  if(name == "manifest.bml") return Emulator::manifest();

  if(name == "boot.cgb-0.rom") {
    return vfs::memory::file::open(Resource::GameBoyColor::BootCGB0, sizeof Resource::GameBoyColor::BootCGB0);
  }

  auto document = BML::unserialize(game.manifest);
  auto programROMSize = document["game/board/memory(content=Program,type=ROM)/size"].natural();
  auto saveRAMVolatile = (bool)document["game/board/memory(Content=Save,type=RAM)/volatile"];

  if(name == "program.rom") {
    return vfs::memory::file::open(game.image.data(), programROMSize);
  }

  if(name == "save.ram" && !saveRAMVolatile) {
    auto location = locate(game.location, ".sav", Emulator::GameFolder);
    if(auto result = vfs::fs::file::open(location, mode)) return result;
  }

  return {};
}
