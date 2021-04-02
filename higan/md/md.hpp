#pragma once

//license: GPLv3
//started: 2016-07-08

#include <emulator/emulator.hpp>
#include <component/audio/sn76489/sn76489.hpp>

namespace higan::MegaDrive {
  #include <emulator/inline.hpp>

  struct Region {
    inline static auto NTSCJ() -> bool;
    inline static auto NTSCU() -> bool;
    inline static auto PAL() -> bool;
  };

  inline static auto MegaCD() -> bool;

  #include <md/controller/controller.hpp>

  #include <md/cpu/cpu.hpp>
  #include <md/apu/apu.hpp>
  #include <md/vdp/vdp.hpp>
  #include <md/psg/psg.hpp>
  #include <md/ym2612/ym2612.hpp>

  //#include <md/mcd/mcd.hpp>

  #include <md/system/system.hpp>
  #include <md/cartridge/cartridge.hpp>
  #include <md/expansion/expansion.hpp>
}

#include <md/interface/interface.hpp>
