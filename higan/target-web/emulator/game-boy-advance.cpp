#include <gba/interface/interface.hpp>

struct GameBoyAdvance : Emulator {
  GameBoyAdvance();
  auto load() -> bool override;
  auto open(higan::Node::Object, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> override;
};

GameBoyAdvance::GameBoyAdvance() {
  interface = new higan::GameBoyAdvance::GameBoyAdvanceInterface;
  name = "Game Boy Advance";
  extensions = {"gba"};
  ports = {};
  buttons = {
      "Up", "Down", "Left", "Right", 
      "B", "A", "L", "R", 
      "Select", "Start"
  };

  firmware.append({"BIOS", "World", "fd2547724b505f487e6dcb29ec2ecff3af35a841a77ab2e85fd87350abd36570"});
}

auto GameBoyAdvance::load() -> bool {
  if(!file::exists(firmware[0].location)) {
    errorFirmwareRequired(firmware[0]);
    return false;
  }

  if(auto port = root->find<higan::Node::Port>("Cartridge Slot")) {
    auto peripheral = port->allocate();
    port->connect(peripheral);
  }

  return true;
}

auto GameBoyAdvance::open(higan::Node::Object node, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> {
  if(name == "manifest.bml") return Emulator::manifest();

  if(name == "bios.rom") {
    return loadFirmware(firmware[0]);
  }

  auto document = BML::unserialize(game.manifest);
  auto programROMSize = document["game/board/memory(content=Program,type=ROM)/size"].natural();
  auto saveRAMVolatile = (bool)document["game/board/memory(Content=Save)/volatile"];

  if(name == "program.rom") {
    return vfs::memory::file::open(game.image.data(), programROMSize);
  }

  if(name == "save.ram" || name == "save.eeprom" || name == "save.flash") {
    if(saveRAMVolatile) return {};
    auto location = locate(game.location, ".sav", Emulator::GameFolder);
    if(auto result = vfs::fs::file::open(location, mode)) return result;
  }

  return {};
}