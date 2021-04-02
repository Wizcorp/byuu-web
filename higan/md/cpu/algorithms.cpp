template<uint Size, bool extend> auto CPU::ADD(uint32 source, uint32 target) -> uint32 {
  auto result = (uint64)source + target;
  if(extend) result += r.x;

  r.c = sign<Size>(result >> 1) < 0;
  r.v = sign<Size>(~(target ^ source) & (target ^ result)) < 0;
  r.z = clip<Size>(result) ? 0 : (extend ? r.z : 1);
  r.n = sign<Size>(result) < 0;
  r.x = r.c;

  return clip<Size>(result);
}

template<uint Size> auto CPU::AND(uint32 source, uint32 target) -> uint32 {
  uint32 result = target & source;

  r.c = 0;
  r.v = 0;
  r.z = clip<Size>(result) == 0;
  r.n = sign<Size>(result) < 0;

  return clip<Size>(result);
}

template<uint Size> auto CPU::ASL(uint32 result, uint shift) -> uint32 {
  bool carry = false;
  uint32 overflow = 0;
  for(auto _ : range(shift)) {
    carry = result & msb<Size>();
    uint32 before = result;
    result <<= 1;
    overflow |= before ^ result;
  }

  r.c = carry;
  r.v = sign<Size>(overflow) < 0;
  r.z = clip<Size>(result) == 0;
  r.n = sign<Size>(result) < 0;
  if(shift) r.x = r.c;

  return clip<Size>(result);
}

template<uint Size> auto CPU::ASR(uint32 result, uint shift) -> uint32 {
  bool carry = false;
  uint32 overflow = 0;
  for(auto _ : range(shift)) {
    carry = result & lsb<Size>();
    uint32 before = result;
    result = sign<Size>(result) >> 1;
    overflow |= before ^ result;
  }

  r.c = carry;
  r.v = sign<Size>(overflow) < 0;
  r.z = clip<Size>(result) == 0;
  r.n = sign<Size>(result) < 0;
  if(shift) r.x = r.c;

  return clip<Size>(result);
}

template<uint Size> auto CPU::CMP(uint32 source, uint32 target) -> uint32 {
  auto result = (uint64)target - source;

  r.c = sign<Size>(result >> 1) < 0;
  r.v = sign<Size>((target ^ source) & (target ^ result)) < 0;
  r.z = clip<Size>(result) == 0;
  r.n = sign<Size>(result) < 0;

  return clip<Size>(result);
}

template<uint Size> auto CPU::EOR(uint32 source, uint32 target) -> uint32 {
  uint32 result = target ^ source;

  r.c = 0;
  r.v = 0;
  r.z = clip<Size>(result) == 0;
  r.n = sign<Size>(result) < 0;

  return clip<Size>(result);
}

template<uint Size> auto CPU::LSL(uint32 result, uint shift) -> uint32 {
  bool carry = false;
  for(auto _ : range(shift)) {
    carry = result & msb<Size>();
    result <<= 1;
  }

  r.c = carry;
  r.v = 0;
  r.z = clip<Size>(result) == 0;
  r.n = sign<Size>(result) < 0;
  if(shift) r.x = r.c;

  return clip<Size>(result);
}

template<uint Size> auto CPU::LSR(uint32 result, uint shift) -> uint32 {
  bool carry = false;
  for(auto _ : range(shift)) {
    carry = result & lsb<Size>();
    result >>= 1;
  }

  r.c = carry;
  r.v = 0;
  r.z = clip<Size>(result) == 0;
  r.n = sign<Size>(result) < 0;
  if(shift) r.x = r.c;

  return clip<Size>(result);
}

template<uint Size> auto CPU::OR(uint32 source, uint32 target) -> uint32 {
  auto result = target | source;

  r.c = 0;
  r.v = 0;
  r.z = clip<Size>(result) == 0;
  r.n = sign<Size>(result) < 0;

  return clip<Size>(result);
}

template<uint Size> auto CPU::ROL(uint32 result, uint shift) -> uint32 {
  bool carry = false;
  for(auto _ : range(shift)) {
    carry = result & msb<Size>();
    result = result << 1 | carry;
  }

  r.c = carry;
  r.v = 0;
  r.z = clip<Size>(result) == 0;
  r.n = sign<Size>(result) < 0;

  return clip<Size>(result);
}

template<uint Size> auto CPU::ROR(uint32 result, uint shift) -> uint32 {
  bool carry = false;
  for(auto _ : range(shift)) {
    carry = result & lsb<Size>();
    result >>= 1;
    if(carry) result |= msb<Size>();
  }

  r.c = carry;
  r.v = 0;
  r.z = clip<Size>(result) == 0;
  r.n = sign<Size>(result) < 0;

  return clip<Size>(result);
}

template<uint Size> auto CPU::ROXL(uint32 result, uint shift) -> uint32 {
  bool carry = r.x;
  for(auto _ : range(shift)) {
    bool extend = carry;
    carry = result & msb<Size>();
    result = result << 1 | extend;
  }

  r.c = carry;
  r.v = 0;
  r.z = clip<Size>(result) == 0;
  r.n = sign<Size>(result) < 0;
  r.x = r.c;

  return clip<Size>(result);
}

template<uint Size> auto CPU::ROXR(uint32 result, uint shift) -> uint32 {
  bool carry = r.x;
  for(auto _ : range(shift)) {
    bool extend = carry;
    carry = result & lsb<Size>();
    result >>= 1;
    if(extend) result |= msb<Size>();
  }

  r.c = carry;
  r.v = 0;
  r.z = clip<Size>(result) == 0;
  r.n = sign<Size>(result) < 0;
  r.x = r.c;

  return clip<Size>(result);
}

template<uint Size, bool extend> auto CPU::SUB(uint32 source, uint32 target) -> uint32 {
  auto result = (uint64)target - source;
  if(extend) result -= r.x;

  r.c = sign<Size>(result >> 1) < 0;
  r.v = sign<Size>((target ^ source) & (target ^ result)) < 0;
  r.z = clip<Size>(result) ? 0 : (extend ? r.z : 1);
  r.n = sign<Size>(result) < 0;
  r.x = r.c;

  return result;
}
