#include <fc/interface/interface.hpp>

struct Famicom : Emulator {
  Famicom();
  auto input(higan::Node::Input node) -> void;
  auto load() -> bool override;
  auto open(higan::Node::Object, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> override;
};

struct FamicomDiskSystem : Emulator {
  FamicomDiskSystem();
  auto input(higan::Node::Input node) -> void;
  auto load() -> bool override;
  auto open(higan::Node::Object, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> override;
  auto notify(const string& message) -> void override;

  vector<uint8_t> diskSide[4];
};

Famicom::Famicom() {
  interface = new higan::Famicom::FamicomInterface;
  name = "Famicom";
  extensions = {"nes"};
  ports = {
    "Controller Port 1",
    "Controller Port 2"
  };
  buttons = {
    "Up", "Down", "Left", "Right", 
    "B", "A", "Select", "Start", 
    "Microphone"
  };
}

auto Famicom::input(higan::Node::Input node) -> void {
  if(node->name() == "Microphone") {
    if(auto buttonMap = buttonMaps.find("Controller Port 1")) {
      auto button = node->cast<higan::Node::Button>();
      button->setValue(buttonMap.get()["Microphone"]);
    }
  } else {
    return Emulator::input(node);
  }
}

auto Famicom::load() -> bool {
  if(auto port = root->find<higan::Node::Port>("Cartridge Slot")) {
    auto peripheral = port->allocate();
    port->connect(peripheral);
  }

  return true;
}

auto Famicom::open(higan::Node::Object node, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> {
  if(name == "manifest.bml") return Emulator::manifest();

  auto document = BML::unserialize(game.manifest);
  auto iNESROMSize = document["game/board/memory(content=iNES,type=ROM)/size"].natural();
  auto programROMSize = document["game/board/memory(content=Program,type=ROM)/size"].natural();
  auto characterROMSize = document["game/board/memory(content=Character,type=ROM)/size"].natural();
  auto programRAMVolatile = (bool)document["game/board/memory(content=Program,type=RAM)/volatile"];

  if(name == "program.rom") {
    return vfs::memory::file::open(game.image.data() + iNESROMSize, programROMSize);
  }

  if(name == "character.rom") {
    return vfs::memory::file::open(game.image.data() + iNESROMSize + programROMSize, characterROMSize);
  }

  if(name == "save.ram" && !programRAMVolatile) {
    auto location = locate(game.location, ".sav", Emulator::GameFolder);
    if(auto result = vfs::fs::file::open(location, mode)) return result;
  }

  if(name == "save.eeprom") {
    auto location = locate(game.location, ".sav", Emulator::GameFolder);
    if(auto result = vfs::fs::file::open(location, mode)) return result;
  }

  return {};
}

FamicomDiskSystem::FamicomDiskSystem() {
  interface = new higan::Famicom::FamicomInterface;
  name = "Famicom Disk System";
  extensions = {"fds"};
  ports = {
    "Controller Port 1",
    "Controller Port 2"
  };
  buttons = {
    "Up", "Down", "Left", "Right", 
    "B", "A", "Select", "Start", 
    "Microphone"
  };

  firmware.append({"BIOS", "Japan"});
}

auto FamicomDiskSystem::input(higan::Node::Input node) -> void {
  if(node->name() == "Microphone") {
    if(auto buttonMap = buttonMaps.find("Controller Port 1")) {
      auto button = node->cast<higan::Node::Button>();
      button->setValue(buttonMap.get()["Microphone"]);
    }
  } else {
    return Emulator::input(node);
  }
}

auto FamicomDiskSystem::load() -> bool {
  if(!file::exists(firmware[0].location)) {
    errorFirmwareRequired(firmware[0]);
    return false;
  }

  for(auto& media : icarus::media) {
    if(media->name() != "Famicom Disk") continue;
    if(auto famicomDisk = media.cast<icarus::FamicomDisk>()) {
      if(game.image.size() % 65500 == 16) {
        //iNES and fwNES headers are unnecessary
        memory::move(&game.image[0], &game.image[16], game.image.size() - 16);
        game.image.resize(game.image.size() - 16);
      }
      array_view<uint8_t> view = game.image;
      uint index = 0;
      while(auto output = famicomDisk->transform(view)) {
        diskSide[index++] = output;
        view += 65500;
        if(index >= 4) break;
      }
    }
  }

  if(auto port = root->find<higan::Node::Port>("Cartridge Slot")) {
    auto peripheral = port->allocate();
    port->connect(peripheral);
  }

  if(auto port = root->scan<higan::Node::Port>("Disk Slot")) {
    auto peripheral = port->allocate();
    port->connect(peripheral);
  }

  if(auto node = root->scan<higan::Node::String>("State")) {
    node->setValue("Disk 1: Side A");
  }

  return true;
}

auto FamicomDiskSystem::open(higan::Node::Object node, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> {
  if(node->name() == "Famicom") {
    if(name == "manifest.bml") {
      for(auto& media : icarus::media) {
        if(media->name() != "Famicom") continue;
        if(auto cartridge = media.cast<icarus::Cartridge>()) {
          if(auto image = loadFirmware(firmware[0])) {
            vector<uint8_t> bios;
            bios.resize(image->size());
            image->read(bios.data(), bios.size());
            auto manifest = cartridge->manifest(bios, firmware[0].location);
            return vfs::memory::file::open(manifest.data<uint8_t>(), manifest.size());
          }
        }
      }
    }

    if(name == "program.rom") {
      return loadFirmware(firmware[0]);
    }
  }

  if(node->name() == "Famicom Disk") {
    if(name == "manifest.bml") {
      for(auto& media : icarus::media) {
        if(media->name() != "Famicom Disk") continue;
        if(auto floppyDisk = media.cast<icarus::FloppyDisk>()) {
          game.manifest = floppyDisk->manifest(game.image, game.location);
        }
      }
      return vfs::memory::file::open(game.manifest.data<uint8_t>(), game.manifest.size());
    }

    if(name == "disk1.sideA") {
      auto location = locate(game.location, ".1A.sav", Emulator::GameFolder);
      if(auto result = vfs::fs::file::open(location, mode)) return result;
      if(mode == vfs::file::mode::read) return vfs::memory::file::open(diskSide[0].data(), diskSide[0].size());
    }

    if(name == "disk1.sideB") {
      auto location = locate(game.location, ".1B.sav", Emulator::GameFolder);
      if(auto result = vfs::fs::file::open(location, mode)) return result;
      if(mode == vfs::file::mode::read) return vfs::memory::file::open(diskSide[1].data(), diskSide[1].size());
    }

    if(name == "disk2.sideA") {
      auto location = locate(game.location, ".2A.sav", Emulator::GameFolder);
      if(auto result = vfs::fs::file::open(location, mode)) return result;
      if(mode == vfs::file::mode::read) return vfs::memory::file::open(diskSide[2].data(), diskSide[2].size());
    }

    if(name == "disk2.sideB") {
      auto location = locate(game.location, ".2B.sav", Emulator::GameFolder);
      if(auto result = vfs::fs::file::open(location, mode)) return result;
      if(mode == vfs::file::mode::read) return vfs::memory::file::open(diskSide[3].data(), diskSide[3].size());
    }
  }

  return {};
}

auto FamicomDiskSystem::notify(const string& message) -> void {
  if(auto node = root->scan<higan::Node::String>("State")) {
    node->setValue(message);
  }
}
