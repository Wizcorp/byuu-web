//JALECO-JF-(23A,24A,25,27B,29A,37,40)
//todo: uPD7756 ADPCM unsupported

struct JalecoJF : Board {
  JalecoJF(Markup::Node& document) : Board(document) {
  }

  auto main() -> void override {
    if(irqEnable) {
      if(!irqCounter--) {
        irqLine = 1;
        switch(irqMode) {
        case 0: irqCounter = uint16(irqReload); break;
        case 1: irqCounter = uint12(irqReload); break;
        case 2: irqCounter =  uint8(irqReload); break;
        case 3: irqCounter =  uint4(irqReload); break;
        }
      }
    }
    cpu.irqLine(irqLine);
    tick();
  }

  auto readPRG(uint address) -> uint8 {
    if(address < 0x6000) return cpu.mdr();

    if(address < 0x8000) {
      // if(!prgram) return cpu.mdr();
      return prgram.read((uint13)address);
    }

    uint6 bank, banks = prgrom.size >> 13;
    switch(address & 0xe000) {
    case 0x8000: bank = programBank[0]; break;
    case 0xa000: bank = programBank[1]; break;
    case 0xc000: bank = programBank[2]; break;
    case 0xe000: bank = banks - 1; break;
    }
    address = bank << 13 | (uint13)address;
    return prgrom.read(address);
  }

  auto writePRG(uint address, uint8 data) -> void {
    if(address < 0x6000) return;

    if(address < 0x8000) {
      // if(!prgram) return;
      return prgram.write((uint13)address, data);
    }

    switch(address & 0xf003) {
    case 0x8000: programBank[0].bit(0,3) = data.bit(0,3); break;
    case 0x8001: programBank[0].bit(4,5) = data.bit(0,1); break;
    case 0x8002: programBank[1].bit(0,3) = data.bit(0,3); break;
    case 0x8003: programBank[1].bit(4,5) = data.bit(0,1); break;
    case 0x9000: programBank[2].bit(0,3) = data.bit(0,3); break;
    case 0x9001: programBank[2].bit(4,5) = data.bit(0,1); break;
    case 0xa000: characterBank[0].bit(0,3) = data.bit(0,3); break;
    case 0xa001: characterBank[0].bit(4,7) = data.bit(0,3); break;
    case 0xa002: characterBank[1].bit(0,3) = data.bit(0,3); break;
    case 0xa003: characterBank[1].bit(4,7) = data.bit(0,3); break;
    case 0xb000: characterBank[2].bit(0,3) = data.bit(0,3); break;
    case 0xb001: characterBank[2].bit(4,7) = data.bit(0,3); break;
    case 0xb002: characterBank[3].bit(0,3) = data.bit(0,3); break;
    case 0xb003: characterBank[3].bit(4,7) = data.bit(0,3); break;
    case 0xc000: characterBank[4].bit(0,3) = data.bit(0,3); break;
    case 0xc001: characterBank[4].bit(4,7) = data.bit(0,3); break;
    case 0xc002: characterBank[5].bit(0,3) = data.bit(0,3); break;
    case 0xc003: characterBank[5].bit(4,7) = data.bit(0,3); break;
    case 0xd000: characterBank[6].bit(0,3) = data.bit(0,3); break;
    case 0xd001: characterBank[6].bit(4,7) = data.bit(0,3); break;
    case 0xd002: characterBank[7].bit(0,3) = data.bit(0,3); break;
    case 0xd003: characterBank[7].bit(4,7) = data.bit(0,3); break;
    case 0xe000: irqReload.bit( 0, 3) = data.bit(0,3); break;
    case 0xe001: irqReload.bit( 4, 7) = data.bit(0,3); break;
    case 0xe002: irqReload.bit( 8,11) = data.bit(0,3); break;
    case 0xe003: irqReload.bit(12,15) = data.bit(0,3); break;
    case 0xf000: irqCounter = irqReload; irqLine = 0; break;
    case 0xf001: irqEnable = data.bit(0); irqMode = data.bit(1,3); irqLine = 0; break;
    case 0xf002: mirror = data.bit(0,1); break;
    case 0xf003: break;  //uPD7756 ADPCM
    }
  }

  auto addressCIRAM(uint address) const -> uint {
    switch(mirror) {
    case 0: return address >> 1 & 0x0400 | address & 0x03ff;  //horizontal mirroring
    case 1: return address >> 0 & 0x0400 | address & 0x03ff;  //vertical mirroring
    case 2: return 0x0000 | address & 0x03ff;                 //one-screen mirroring (first)
    case 3: return 0x0400 | address & 0x03ff;                 //one-screen mirroring (second)
    }
    unreachable;
  }

  auto addressCHR(uint address) const -> uint {
    uint8 bank = characterBank[address >> 10];
    return bank << 10 | (uint10)address;
  }

  auto readCHR(uint address) -> uint8 {
    if(address & 0x2000) return ppu.readCIRAM(addressCIRAM(address));
    return chrrom.read(addressCHR(address));
    return 0x00;
  }

  auto writeCHR(uint address, uint8 data) -> void {
    if(address & 0x2000) return ppu.writeCIRAM(addressCIRAM(address), data);
  }

  auto power() -> void {
    for(uint n : range(3)) programBank[n] = n;
    for(uint n : range(8)) characterBank[n] = n;
    irqLine = 0;
    irqCounter = 0;
    irqReload = 0;
    irqEnable = 0;
    irqMode = 0;
    mirror = 0;
  }

  auto serialize(serializer& s) -> void {
    if(prgram.size) s.array(prgram.data, prgram.size);
    s.array(programBank);
    s.array(characterBank);
    s.integer(irqLine);
    s.integer(irqCounter);
    s.integer(irqReload);
    s.integer(irqEnable);
    s.integer(irqMode);
    s.integer(mirror);
  }

   uint6 programBank[3];
   uint8 characterBank[8];
   uint1 irqLine;
  uint16 irqCounter;
  uint16 irqReload;
   uint1 irqEnable;
   uint3 irqMode;
   uint2 mirror;
};
