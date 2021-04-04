auto CPU::serialize(serializer& s) -> void {
  s.array(r.d);
  s.array(r.a);
  s.integer(r.sp);
  s.integer(r.pc);

  s.integer(r.c);
  s.integer(r.v);
  s.integer(r.z);
  s.integer(r.n);
  s.integer(r.x);
  s.integer(r.i);
  s.integer(r.s);
  s.integer(r.t);

  s.integer(r.irc);
  s.integer(r.ir);
  s.integer(r.ird);

  s.integer(r.stop);
  s.integer(r.reset);

  Thread::serialize(s);

  ram.serialize(s);

  s.boolean(io.version);
  s.boolean(io.romEnable);
  s.boolean(io.vdpEnable[0]);
  s.boolean(io.vdpEnable[1]);

  s.integer(refresh.ram);
  s.integer(refresh.external);

  s.integer(state.interruptLine);
  s.integer(state.interruptPending);
}
