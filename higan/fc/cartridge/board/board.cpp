#include "bandai-fcg.cpp"
#include "discrete-74x139x74.cpp"
#include "hvc-fmr.cpp"
#include "irem-g101.cpp"
#include "irem-tam-s1.cpp"
#include "jaleco-jf.cpp"
#include "jaleco-jf16.cpp"
#include "konami-vrc1.cpp"
#include "konami-vrc2.cpp"
#include "konami-vrc3.cpp"
#include "konami-vrc4.cpp"
#include "konami-vrc5.cpp"
#include "konami-vrc6.cpp"
#include "konami-vrc7.cpp"
#include "namco-163.cpp"
#include "namco-34xx.cpp"
#include "nes-axrom.cpp"
#include "nes-bnrom.cpp"
#include "nes-cnrom.cpp"
#include "nes-exrom.cpp"
#include "nes-fxrom.cpp"
#include "nes-gxrom.cpp"
#include "nes-hkrom.cpp"
#include "nes-nrom.cpp"
#include "nes-pxrom.cpp"
#include "nes-sxrom.cpp"
#include "nes-txrom.cpp"
#include "nes-uxrom.cpp"
#include "sunsoft-1.cpp"
#include "sunsoft-2.cpp"
#include "sunsoft-4.cpp"
#include "sunsoft-5b.cpp"

Board::Board(Markup::Node document) {
  cartridge.board = this;
  information.type = document["game/board"].text();
}

auto Board::load() -> void {
  auto document = BML::unserialize(cartridge.manifest());

  if(auto memory = document["game/board/memory(type=ROM,content=Program)"]) {
    if(prgrom.size = memory["size"].natural()) prgrom.data = new uint8_t[prgrom.size]();
    if(auto fp = platform->open(cartridge.node, "program.rom", File::Read, File::Required)) {
      fp->read(prgrom.data, min(prgrom.size, fp->size()));
    }
  }

  if(auto memory = document["game/board/memory(type=RAM,content=Save)"]) {
    if(prgram.size = memory["size"].natural()) prgram.data = new uint8_t[prgram.size](), prgram.writable = true;
    if(!memory["volatile"]) {
      if(auto fp = platform->open(cartridge.node, "save.ram", File::Read)) {
        fp->read(prgram.data, min(prgram.size, fp->size()));
      }
    }
  }

  if(auto memory = document["game/board/memory(type=ROM,content=Character)"]) {
    if(chrrom.size = memory["size"].natural()) chrrom.data = new uint8_t[chrrom.size]();
    if(auto fp = platform->open(cartridge.node, "character.rom", File::Read, File::Required)) {
      fp->read(chrrom.data, min(chrrom.size, fp->size()));
    }
  }

  if(auto memory = document["game/board/memory(type=RAM,content=Character)"]) {
    if(chrram.size = memory["size"].natural()) chrram.data = new uint8_t[chrram.size](), chrram.writable = true;
    if(!memory["volatile"]) {
      if(auto fp = platform->open(cartridge.node, "character.ram", File::Read)) {
        fp->read(chrram.data, min(chrram.size, fp->size()));
      }
    }
  }
}

auto Board::save() -> void {
  auto document = BML::unserialize(cartridge.manifest());

  if(auto memory = document["game/board/memory(type=RAM,content=Save)"]) {
    if(!memory["volatile"]) {
      if(auto fp = platform->open(cartridge.node, "save.ram", File::Write)) {
        fp->write(prgram.data, prgram.size);
      }
    }
  }

  if(auto memory = document["game/board/memory(type=RAM,content=Character)"]) {
    if(!memory["volatile"]) {
      if(auto fp = platform->open(cartridge.node, "character.ram", File::Write)) {
        fp->write(chrram.data, chrram.size);
      }
    }
  }
}

auto Board::Memory::read(uint addr) const -> uint8 {
  return data[mirror(addr, size)];
}

auto Board::Memory::write(uint addr, uint8 byte) -> void {
  if(writable) data[mirror(addr, size)] = byte;
}

auto Board::mirror(uint addr, uint size) -> uint {
  uint base = 0;
  if(size) {
    uint mask = 1 << 23;
    while(addr >= size) {
      while(!(addr & mask)) mask >>= 1;
      addr -= mask;
      if(size > mask) {
        size -= mask;
        base += mask;
      }
      mask >>= 1;
    }
    base += addr;
  }
  return base;
}

auto Board::main() -> void {
  cartridge.step(cartridge.rate() * 4095);
  tick();
}

auto Board::tick(uint clocks) -> void {
  cartridge.step(cartridge.rate() * clocks);
  cartridge.synchronize(cpu);
}

auto Board::readCHR(uint addr) -> uint8 {
  if(chrram.size) return chrram.data[mirror(addr, chrram.size)];
  if(chrrom.size) return chrrom.data[mirror(addr, chrrom.size)];
  return 0u;
}

auto Board::writeCHR(uint addr, uint8 data) -> void {
  if(chrram.size) chrram.data[mirror(addr, chrram.size)] = data;
}

auto Board::power() -> void {
}

auto Board::serialize(serializer& s) -> void {
  if(prgram.size) s.array(prgram.data, prgram.size);
  if(chrram.size) s.array(chrram.data, chrram.size);
}

auto Board::load(string manifest) -> Board* {
  auto document = BML::unserialize(manifest);

  string type = document["game/board"].text();

  if(type == "BANDAI-FCG"  ) return new BandaiFCG(document);
  if(type == "DISCRETE-74x139x74") return new DISCRETE74x139x74(document);

  if(type == "IREM-G101"     ) return new IremG101(document);
  if(type == "IREM-TAM-S1"   ) return new IremTamS1(document);

  if(type == "HVC-FMR"     ) return new HVC_FMR(document);

  if(type == "JALECO-JF"  ) return new JalecoJF(document);
  if(type == "JALECO-JF16") return new JalecoJF16(document);

  if(type == "KONAMI-VRC-1") return new KonamiVRC1(document);
  if(type == "KONAMI-VRC-2") return new KonamiVRC2(document);
  if(type == "KONAMI-VRC-3") return new KonamiVRC3(document);
  if(type == "KONAMI-VRC-4") return new KonamiVRC4(document);
  if(type == "KONAMI-VRC-5") return new KonamiVRC5(document);
  if(type == "KONAMI-VRC-6") return new KonamiVRC6(document);
  if(type == "KONAMI-VRC-7") return new KonamiVRC7(document);

  if(type == "NAMCOT-129" ) return new Namco163(document);
  if(type == "NAMCOT-163" ) return new Namco163(document);
  if(type == "NAMCOT-175" ) return new Namco163(document);
  if(type == "NAMCOT-340" ) return new Namco163(document);
  if(type == "NAMCOT-3401") return new Namco34xx(document);
  if(type == "NAMCOT-3406") return new Namco34xx(document);
  if(type == "NAMCOT-3407") return new Namco34xx(document);
  if(type == "NAMCOT-3413") return new Namco34xx(document);
  if(type == "NAMCOT-3414") return new Namco34xx(document);
  if(type == "NAMCOT-3415") return new Namco34xx(document);
  if(type == "NAMCOT-3416") return new Namco34xx(document);
  if(type == "NAMCOT-3417") return new Namco34xx(document);
  if(type == "NAMCOT-3425") return new Namco34xx(document);
  if(type == "NAMCOT-3443") return new Namco34xx(document);
  if(type == "NAMCOT-3446") return new Namco34xx(document);
  if(type == "NAMCOT-3451") return new Namco34xx(document);
  if(type == "NAMCOT-3453") return new Namco34xx(document);

  if(type == "NES-AMROM"   ) return new NES_AxROM(document);
  if(type == "NES-ANROM"   ) return new NES_AxROM(document);
  if(type == "NES-AN1ROM"  ) return new NES_AxROM(document);
  if(type == "NES-AOROM"   ) return new NES_AxROM(document);

  if(type == "NES-BNROM"   ) return new NES_BNROM(document);

  if(type == "NES-CNROM"   ) return new NES_CNROM(document);

  if(type == "NES-EKROM"   ) return new NES_ExROM(document);
  if(type == "NES-ELROM"   ) return new NES_ExROM(document);
  if(type == "NES-ETROM"   ) return new NES_ExROM(document);
  if(type == "NES-EWROM"   ) return new NES_ExROM(document);

  if(type == "NES-FJROM"   ) return new NES_FxROM(document);
  if(type == "NES-FKROM"   ) return new NES_FxROM(document);

  if(type == "NES-GNROM"   ) return new NES_GxROM(document);
  if(type == "NES-MHROM"   ) return new NES_GxROM(document);

  if(type == "NES-HKROM"   ) return new NES_HKROM(document);

  if(type == "NES-NROM"    ) return new NES_NROM(document);
  if(type == "NES-NROM-128") return new NES_NROM(document);
  if(type == "NES-NROM-256") return new NES_NROM(document);

  if(type == "NES-PEEOROM" ) return new NES_PxROM(document);
  if(type == "NES-PNROM"   ) return new NES_PxROM(document);

  if(type == "NES-SAROM"   ) return new NES_SxROM(document);
  if(type == "NES-SBROM"   ) return new NES_SxROM(document);
  if(type == "NES-SCROM"   ) return new NES_SxROM(document);
  if(type == "NES-SC1ROM"  ) return new NES_SxROM(document);
  if(type == "NES-SEROM"   ) return new NES_SxROM(document);
  if(type == "NES-SFROM"   ) return new NES_SxROM(document);
  if(type == "NES-SFEXPROM") return new NES_SxROM(document);
  if(type == "NES-SGROM"   ) return new NES_SxROM(document);
  if(type == "NES-SHROM"   ) return new NES_SxROM(document);
  if(type == "NES-SH1ROM"  ) return new NES_SxROM(document);
  if(type == "NES-SIROM"   ) return new NES_SxROM(document);
  if(type == "NES-SJROM"   ) return new NES_SxROM(document);
  if(type == "NES-SKROM"   ) return new NES_SxROM(document);
  if(type == "NES-SLROM"   ) return new NES_SxROM(document);
  if(type == "NES-SL1ROM"  ) return new NES_SxROM(document);
  if(type == "NES-SL2ROM"  ) return new NES_SxROM(document);
  if(type == "NES-SL3ROM"  ) return new NES_SxROM(document);
  if(type == "NES-SLRROM"  ) return new NES_SxROM(document);
  if(type == "NES-SMROM"   ) return new NES_SxROM(document);
  if(type == "NES-SNROM"   ) return new NES_SxROM(document);
  if(type == "NES-SOROM"   ) return new NES_SxROM(document);
  if(type == "NES-SUROM"   ) return new NES_SxROM(document);
  if(type == "NES-SXROM"   ) return new NES_SxROM(document);

  if(type == "NES-TBROM"   ) return new NES_TxROM(document);
  if(type == "NES-TEROM"   ) return new NES_TxROM(document);
  if(type == "NES-TFROM"   ) return new NES_TxROM(document);
  if(type == "NES-TGROM"   ) return new NES_TxROM(document);
  if(type == "NES-TKROM"   ) return new NES_TxROM(document);
  if(type == "NES-TKSROM"  ) return new NES_TxROM(document);
  if(type == "NES-TLROM"   ) return new NES_TxROM(document);
  if(type == "NES-TL1ROM"  ) return new NES_TxROM(document);
  if(type == "NES-TL2ROM"  ) return new NES_TxROM(document);
  if(type == "NES-TLSROM"  ) return new NES_TxROM(document);
  if(type == "NES-TNROM"   ) return new NES_TxROM(document);
  if(type == "NES-TQROM"   ) return new NES_TxROM(document);
  if(type == "NES-TR1ROM"  ) return new NES_TxROM(document);
  if(type == "NES-TSROM"   ) return new NES_TxROM(document);
  if(type == "NES-TVROM"   ) return new NES_TxROM(document);

  if(type == "NES-UNROM"   ) return new NES_UxROM(document);
  if(type == "NES-UOROM"   ) return new NES_UxROM(document);

  if(type == "SUNSOFT-1"   ) return new Sunsoft1(document);
  if(type == "SUNSOFT-2"   ) return new Sunsoft2(document);
  if(type == "SUNSOFT-4"  ) return new Sunsoft4(document);
  if(type == "SUNSOFT-5B"  ) return new Sunsoft5B(document);

  return nullptr;
}
