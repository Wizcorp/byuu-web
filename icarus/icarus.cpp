#include "icarus.hpp"

namespace icarus {

vector<shared_pointer<Media>> media;

auto locate(string name) -> string {
  string location = {Path::program(), name};
  if(inode::exists(location)) return location;

  directory::create({Path::userData(), "icarus/"});
  return {Path::userData(), "icarus/", name};
}

auto operator+=(string& lhs, const string& rhs) -> string& {
  lhs.append(rhs);
  return lhs;
}

#include "settings/settings.cpp"

#include "media/media.cpp"
#include "cartridge/cartridge.cpp"
#include "compact-disc/compact-disc.cpp"
#include "floppy-disk/floppy-disk.cpp"

#if !defined(ICARUS_LIBRARY)
#include "program/program.cpp"
#endif

auto construct() -> void {
#if !defined(ICARUS_LIBRARY)
  media.append(new BSMemory);
  media.append(new ColecoVision);
  media.append(new Famicom);
  media.append(new FamicomDisk);
  media.append(new GameBoy);
  media.append(new GameBoyAdvance);
  media.append(new GameBoyColor);
  media.append(new GameGear);
  media.append(new MasterSystem);
  media.append(new MegaCD);
  media.append(new MegaDrive);
  media.append(new MSX);
  media.append(new MSX2);
  media.append(new NeoGeoPocket);
  media.append(new NeoGeoPocketColor);
  media.append(new PCEngine);
  media.append(new PCEngineCD);
  media.append(new PocketChallengeV2);
  media.append(new SC3000);
  media.append(new SG1000);
  media.append(new SufamiTurbo);
  media.append(new SuperFamicom);
  media.append(new SuperGrafx);
  media.append(new WonderSwan);
  media.append(new WonderSwanColor);
#else
#ifdef CORE_FC
  media.append(new Famicom);
  media.append(new FamicomDisk);
#endif

#ifdef CORE_GB
  media.append(new GameBoy);
#endif

#ifdef CORE_GBA
  media.append(new GameBoyAdvance);
#endif

#ifdef CORE_MD
  media.append(new MegaCD);
  media.append(new MegaDrive);
#endif

#ifdef CORE_MS
  media.append(new MasterSystem);
#endif

#ifdef CORE_MSX
  media.append(new MSX);
  media.append(new MSX2);
#endif

#ifdef CORE_NGP
  media.append(new NeoGeoPocket);
  media.append(new NeoGeoPocketColor);
#endif

#ifdef CORE_PCE
  media.append(new PCEngine);
  media.append(new PCEngineCD);
#endif

#ifdef CORE_SFC
  media.append(new SuperFamicom);
#endif

#ifdef CORE_SG
  media.append(new SG1000);
#endif

#ifdef CORE_WS
  media.append(new WonderSwan);
  media.append(new WonderSwanColor);
#endif
#endif

  for(auto& medium : media) medium->construct();
}

#if !defined(ICARUS_LIBRARY)
auto main(Arguments arguments) -> void {
  Application::setName("icarus");

  construct();

  if(auto document = file::read({Path::userSettings(), "icarus/settings.bml"})) {
    settings.unserialize(document);
  }

  if(arguments.take("--name")) {
    return print("icarus");
  }

  if(string system; arguments.take("--system", system)) {
    for(auto& medium : media) {
      if(medium->name() != system) continue;

      if(string manifest; arguments.take("--manifest", manifest)) {
        return print(medium->manifest(manifest));
      }

      if(string import; arguments.take("--import", import)) {
        return (void)medium->import(import);
      }

      if(arguments.take("--import")) {
        if(auto import = BrowserDialog()
        .setTitle({"Import ", system, " Game"})
        .setPath(settings.recent)
        .setAlignment(Alignment::Center)
        .openFile()
        ) {
          if(auto error = medium->import(import)) {
            MessageDialog()
            .setTitle("Error")
            .setAlignment(Alignment::Center)
            .setText({"Failed to import: ", Location::file(import), "\n\nError: ", error, "."})
            .error();
          }
        }
        return;
      }
    }
  }

  Instances::programWindow.construct();

  #if defined(PLATFORM_MACOS)
  Application::Cocoa::onAbout([&] { programWindow.aboutAction.doActivate(); });
  Application::Cocoa::onPreferences([&] {});
  Application::Cocoa::onQuit([&] { Application::quit(); });
  #endif

  programWindow.setVisible();
  Application::run();

  directory::create({Path::userSettings(), "icarus/"});
  file::write({Path::userSettings(), "icarus/settings.bml"}, settings.serialize());
}
#endif
}

#if !defined(ICARUS_LIBRARY)
#include <nall/main.hpp>
auto nall::main(Arguments arguments) -> void {
  icarus::main(arguments);
}
#endif
