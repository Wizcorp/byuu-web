auto APU::serialize(serializer& s) -> void {
  Thread::serialize(s);

  apu_snapshot_t snapshot;
  if(s.mode() == serializer::Save) {
    nes_apu.save_snapshot(&snapshot);
    serialize_snapshot(s, snapshot);

  } else if(s.mode() == serializer::Load) {
    serialize_snapshot(s, snapshot);
    nes_apu.load_snapshot(snapshot);
  }
}

auto APU::serialize_snapshot(serializer& s, apu_snapshot_t& snapshot) -> void {
  s.array(snapshot.w40xx);
  s.integer(snapshot.w4015);
  s.integer(snapshot.w4017);
  s.integer(snapshot.delay);
  s.integer(snapshot.step);
  s.integer(snapshot.irq_flag);

  s.integer(snapshot.square1.delay);
  s.array(snapshot.square1.env);
  s.integer(snapshot.square1.length);
  s.integer(snapshot.square1.phase);
  s.integer(snapshot.square1.swp_delay);
  s.integer(snapshot.square1.swp_reset);

  s.integer(snapshot.square2.delay);
  s.array(snapshot.square2.env);
  s.integer(snapshot.square2.length);
  s.integer(snapshot.square2.phase);
  s.integer(snapshot.square2.swp_delay);
  s.integer(snapshot.square2.swp_reset);

  s.integer(snapshot.triangle.delay);
  s.integer(snapshot.triangle.length);
  s.integer(snapshot.triangle.phase);
  s.integer(snapshot.triangle.linear_counter);
  s.integer(snapshot.triangle.linear_mode);

  s.integer(snapshot.noise.delay);
  s.array(snapshot.noise.env);
  s.integer(snapshot.noise.length);
  s.integer(snapshot.noise.shift_reg);

  s.integer(snapshot.dmc.delay);
  s.integer(snapshot.dmc.remain);
  s.integer(snapshot.dmc.addr);
  s.integer(snapshot.dmc.buf);
  s.integer(snapshot.dmc.bits_remain);
  s.integer(snapshot.dmc.bits);
  s.integer(snapshot.dmc.buf_empty);
  s.integer(snapshot.dmc.silence);
  s.integer(snapshot.dmc.irq_flag);
}