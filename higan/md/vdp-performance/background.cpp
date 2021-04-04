template<bool interlace> auto VDP::Background::renderScreen(const uint from, const uint to) -> void {
  constexpr uint mask[] = {0u, 7u, ~7u, ~0u};
  const uint15 scrollAddress = io.horizontalScrollAddress + ((vdp.state.vcounter & mask[io.horizontalScrollMode]) << 1);
  uint x = 0 - vdp.vram.memory[scrollAddress + (id == ID::PlaneB)];
  const uint tileShift = interlace ? 7 : 6;

  const auto vsram = &vdp.vsram.memory[id == ID::PlaneB];
  uint y = vdp.state.vcounter;
  if(interlace) y = y << 1 | vdp.state.field;
  y += vdp.vsram.memory[id == ID::PlaneB];
  uint w = from;
  uint tileX = x >> 3 & nametableWidthMask;
  uint tileY = y >> 3 + interlace & nametableHeightMask;
  uint tileY_x_width = tileY * nametableWidth;
  const uint maskY = interlace ? 15 : 7;
  uint address = io.nametableAddress + (tileY_x_width + tileX & 0x0fff);
  uint tileAttributes = vdp.vram.memory[address & 0x7fff];
  uint flipX = tileAttributes & 0x0800 ? 7 : 0;
  uint flipY = tileAttributes & 0x1000 ? maskY : 0;
  uint pixelY = y & maskY ^ flipY;
  auto tileData = &vdp.vram.pixels[(tileAttributes << tileShift) + (pixelY << 3) + (x & 7 ^ flipX) & 0x1ffff];
  int incrementX = flipX ? -1 : +1;

  while(w < to) {
    pixels[w] = *tileData | tileAttributes >> 13;
    tileData += incrementX;

    if((w++ & 15) == 15 && io.verticalScrollMode) {
      y = vdp.state.vcounter;
      if(interlace) y = y << 1 | vdp.state.field;
      y += vsram[w >> 4 << 1];
      if((x++ & 7) == 7) tileX = x >> 3 & nametableWidthMask;
      tileY = y >> 3 + interlace & nametableHeightMask;
      tileY_x_width = tileY * nametableWidth;
      pixelY = y & maskY;
      address = io.nametableAddress + (tileY_x_width + tileX & 0x0fff);
      tileAttributes = vdp.vram.memory[address & 0x7fff];
      flipX = tileAttributes & 0x0800 ? 7 : 0;
      flipY = tileAttributes & 0x1000 ? maskY : 0;
      pixelY = y & maskY ^ flipY;
      tileData = &vdp.vram.pixels[(tileAttributes << tileShift) + (pixelY << 3) + (x & 7 ^ flipX) & 0x1ffff];
      incrementX = flipX ? -1 : +1;
    }

    else if((x++ & 7) == 7) {
      tileX = x >> 3 & nametableWidthMask;
      address = io.nametableAddress + (tileY_x_width + tileX & 0x0fff);
      tileAttributes = vdp.vram.memory[address & 0x7fff];
      flipX = tileAttributes & 0x0800 ? 7 : 0;
      flipY = tileAttributes & 0x1000 ? maskY : 0;
      pixelY = y & maskY ^ flipY;
      tileData = &vdp.vram.pixels[(tileAttributes << tileShift) + (pixelY << 3) + (x & 7 ^ flipX) & 0x1ffff];
      incrementX = flipX ? -1 : +1;
    }
  }
}

template<bool interlace> auto VDP::Background::renderWindow(const uint from, const uint to) -> void {
  const uint tileShift = interlace ? 7 : 6;

  uint y = vdp.state.vcounter;
  if(interlace) y = y << 1 | vdp.state.field;

  uint x = from;
  uint tileX = x >> 3 & widthMask;
  const uint tileY = y >> 3 + interlace & 31;
  const uint tileY_x_width = tileY * widthSize;
  const uint maskY = interlace ? 15 : 7;
  uint address = nametableAddress + (tileY_x_width + tileX & 0x0fff);
  uint tileAttributes = vdp.vram.memory[address & 0x7fff];
  uint flipX = tileAttributes & 0x0800 ? 7 : 0;
  uint flipY = tileAttributes & 0x1000 ? maskY : 0;
  uint pixelY = y & maskY ^ flipY;
  auto tileData = &vdp.vram.pixels[(tileAttributes << tileShift) + (pixelY << 3) + (x & 7 ^ flipX) & 0x1ffff];
  int incrementX = flipX ? -1 : +1;

  while(x < to) {
    vdp.planeA.pixels[x] = *tileData | tileAttributes >> 13;
    tileData += incrementX;

    if((x++ & 7) == 7) {
      tileX = x >> 3 & widthMask;
      address = nametableAddress + (tileY_x_width + tileX & 0x0fff);
      tileAttributes = vdp.vram.memory[address & 0x7fff];
      flipX = tileAttributes & 0x0800 ? 7 : 0;
      flipY = tileAttributes & 0x1000 ? maskY : 0;
      pixelY = y & maskY ^ flipY;
      tileData = &vdp.vram.pixels[(tileAttributes << tileShift) + (pixelY << 3) + (x & 7 ^ flipX) & 0x1ffff];
      incrementX = flipX ? -1 : +1;
    }
  }
}
