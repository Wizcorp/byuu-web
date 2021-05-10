auto VDP::DMA::poll() -> void {
  static bool locked = false;
  if(locked) return;
  locked = true;
  if(cpu.active()) cpu.synchronize(apu, vdp);
  if(apu.active()) apu.synchronize(cpu, vdp);
  while(run());
  locked = false;
}

auto VDP::DMA::run() -> bool {
  if(!io.enable || io.wait) return false;
  if(!vdp.io.command.bit(5)) return false;
  if(io.mode <= 1) return load(), true;
  if(io.mode == 2) return fill(), true;
  if(!vdp.io.command.bit(4)) return false;
  if(io.mode == 3) return copy(), true;
  return false;
}

auto VDP::DMA::load() -> void {
  active = 1;

  auto address = io.mode.bit(0) << 23 | io.source << 1;
  auto data = cpu.read(1, 1, address);
  vdp.writeDataPort(data);

  io.source.bit(0,15)++;
  if(--io.length == 0) {
    vdp.io.command.bit(5) = 0;
    active = 0;
  }
}

//todo: supposedly, this can also write to VSRAM and CRAM (undocumented)
auto VDP::DMA::fill() -> void {
  if(vdp.io.command.bit(0,3) == 1) {
    vdp.vram.writeByte(vdp.io.address, io.fill);
  }

  io.source.bit(0,15)++;
  vdp.io.address += vdp.io.dataIncrement;
  if(--io.length == 0) {
    vdp.io.command.bit(5) = 0;
  }
}

//note: this can only copy to VRAM
auto VDP::DMA::copy() -> void {
  auto data = vdp.vram.readByte(io.source);
  vdp.vram.writeByte(vdp.io.address, data);

  io.source.bit(0,15)++;
  vdp.io.address += vdp.io.dataIncrement;
  if(--io.length == 0) {
    vdp.io.command.bit(5) = 0;
  }
}

auto VDP::DMA::power() -> void {
  active = 0;
  io = {};
}