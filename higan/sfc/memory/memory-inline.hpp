auto Bus::mirror(uint address, uint size) -> uint {
  if(size == 0) return 0;
  uint base = 0;
  uint mask = 1 << 23;
  while(address >= size) {
    while(!(address & mask)) mask >>= 1;
    address -= mask;
    if(size > mask) {
      size -= mask;
      base += mask;
    }
    mask >>= 1;
  }
  return base + address;
}

auto Bus::reduce(uint address, uint mask) -> uint {
  while(mask) {
    uint bits = (mask & -mask) - 1;
    address = (address >> 1 & ~bits) | address & bits;
    mask = (mask & mask - 1) >> 1;
  }
  return address;
}

auto Bus::read(uint24 address, uint8 data) -> uint8 {
  if (fast_read[address>>fast_page_size_bits]) {
    return fast_read[address>>fast_page_size_bits][address];
  }

  return reader[lookup[address]](target[address], data);
}

auto Bus::write(uint24 address, uint8 data) -> void {
  if (fast_write[address>>fast_page_size_bits]) {
    fast_write[address>>fast_page_size_bits][address] = data;
  } else {
    writer[lookup[address]](target[address], data);
  }
}
