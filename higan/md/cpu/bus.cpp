/* byte reads to word memory areas discard the unselected byte inside the M68K core.
 * as an optimization, the entire word is returned anyway.
 * byte writes to word memory areas that are addressable as bytes cannot enjoy this optimization.
 */

auto CPU::read(const uint1 upper, const uint1 lower, const uint24 address, uint16 data) -> uint16 { 
  if ((address & 0xc00000) == 0) {
    return cartridge.read(upper, lower, address, data);
  }

  if ((address & 0xe00000) == 0xe00000) {
    return ram[address >> 1];
  }

  switch(address & 0xf00000) {
    case 0xa00000: case 0xb00000:
      if(address <= 0xa0ffff) {
        auto addrCopy = address;
        if(apu.busStatus()) return data;
        addrCopy.bit(15) = 0;  //a080000-a0ffff mirrors a00000-a07fff
        //word reads load the even input byte into both output bytes
        auto byte = apu.read(addrCopy | !upper);  //upper==0 only on odd byte reads
        return byte << 8 | byte << 0;
      }

      if(address <= 0xbfffff) {
        data = cartridge.readIO(upper, lower, address, data);
        //data = expansion.readIO(upper, lower, address, data);
        data = readIO(upper, lower, address, data);
        return data;
      }
      break;
    case 0xc00000: case 0xd00000: {
      if(address.bit(5,7)) return cpu.ird();  //should deadlock the machine
      if(address.bit(16,18)) return cpu.ird();  //should deadlock the machine
      if(address.bit(2,3) == 3) return cpu.ird();  //should return VDP open bus
      if(address.bit(4)) return cpu.ird();  //reading the PSG should deadlock the machine

      auto addrCopy = address;
      addrCopy.bit(8,15) = 0;  //mirrors
      return vdp.read(addrCopy, data);
    }
  }

  return data;
}

auto CPU::write(const uint1 upper, const uint1 lower, const uint24 address, const uint16 data) -> void {
  if ((address & 0xe00000) == 0xe00000) {
      if(upper) ram[address >> 1].byte(1) = data.byte(1);
      if(lower) ram[address >> 1].byte(0) = data.byte(0);
      return;
  }

  if ((address & 0xc00000) == 0) {
    return cartridge.write(upper, lower, address, data);
  }

  switch(address & 0xf00000) {
    case 0xa00000: case 0xb00000:
      if(address <= 0xa0ffff) {
        auto addrCopy = address;
        if(apu.busStatus()) return;
        addrCopy.bit(15) = 0;  //a08000-a0ffff mirrors a00000-a07fff
        //word writes store the upper input byte into the lower output byte
        return apu.write(addrCopy | !upper, data.byte(upper));  //upper==0 only on odd byte reads
      }

      if(address <= 0xbfffff) {
        cartridge.writeIO(upper, lower, address, data);
        //expansion.writeIO(upper, lower, address, data);
        writeIO(upper, lower, address, data);
        return;
      }
      break;
    case 0xc00000: case 0xd00000: {
      if(address.bit(5,7)) return;  //should deadlock the machine
      if(address.bit(16,18)) return;  //should deadlock the machine
      if(address.bit(4)) {
        if(!lower) return;  //byte writes to even PSG registers has no effect
        return psg.write(data.byte(0));
      }
      
      auto addrCopy = address;
      addrCopy.bit(8,15) = 0;  //mirrors
      return vdp.write(addrCopy, data);
    }
  }
  
}
