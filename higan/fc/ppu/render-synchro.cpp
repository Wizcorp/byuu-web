#include "render-synchro.hpp"

auto PPU::renderScanline() -> void {
  if (callcount > 0) {
    io.lx++;
  }
  
  //Vblank - todo: how would we treat this? - need to run step 341 times in a row... can we optimize this case?
  if (renderState.vBlankCycles) {
    renderState.vBlankCycles--;
    return step();
  }

  if (renderState.scanlineAfterVBlankCycles) {
    renderState.scanlineAfterVBlankCycles = false;
    callcount = 0;
    return scanline();
  }
  
  switch (callcount) {
    //  0
    case 0:
      if(io.ly >= 240 && io.ly <= vlines() - 2) {
        renderState.scanlineAfterVBlankCycles = true;
        renderState.vBlankCycles = 340;
        step();
        break;
      }

      latch.oamIterator = 0;
      latch.oamCounter = 0;
      for(auto n : range(8)) latch.soam[n] = {};
      step();
      break;
    
    //  1-256 - 8 case x 32 = 256
    case RENDER_CASE_1_256_1:  
      // tail end of loop
      latch.nametable = latch.nametable << 8 | renderState.nametable;
      latch.attribute = latch.attribute << 2 | (renderState.attribute & 3);
      latch.tiledataLo = latch.tiledataLo << 8 | renderState.tiledataLo;
      latch.tiledataHi = latch.tiledataHi << 8 | renderState.tiledataHi;
      // Do not break, continue to beginning of loop code
      
    case 1:
      renderState.nametable = loadCHR(0x2000 | (uint12)io.v.address);
      renderState.tileaddr = io.bgAddress | renderState.nametable << 4 | io.v.fineY;
      renderPixel();
      step();
      break;
    
    case RENDER_CASE_1_256_2:
      renderPixel();
      step();
      break;

    case RENDER_CASE_1_256_3:
      renderState.attribute = loadCHR(0x23c0 | io.v.nametable << 10 | (io.v.tileY >> 2) << 3 | io.v.tileX >> 2);
      if(io.v.tileY & 2) renderState.attribute >>= 4;
      if(io.v.tileX & 2) renderState.attribute >>= 2;
      renderPixel();
      step();
      break;

    case 244:
      if(enable() && ++io.v.fineY == 0 && ++io.v.tileY == 30) io.v.nametableY++, io.v.tileY = 0;
      // do not break, continue into natural case group

    case RENDER_CASE_1_256_4:
      if(enable() && ++io.v.tileX == 0) io.v.nametableX++;
      renderPixel();
      renderSprite();
      step();
      break;

    case RENDER_CASE_1_256_5:
      renderState.tiledataLo = loadCHR(renderState.tileaddr + 0);
      renderPixel();
      step();
      break;

    case RENDER_CASE_1_256_6:
      renderPixel();
      step();
      break;

    case RENDER_CASE_1_256_7:
      renderState.tiledataHi = loadCHR(renderState.tileaddr + 8);
      renderPixel();
      step();
      break;

    case RENDER_CASE_1_256_8:
      renderPixel();
      renderSprite();
      step();
      break;

    //257-320 - 8 cases x 8 = 64
    case 257:
      // tail end of previous loop (RENDER_CASE_1_256)
      latch.nametable = latch.nametable << 8 | renderState.nametable;
      latch.attribute = latch.attribute << 2 | (renderState.attribute & 3);
      latch.tiledataLo = latch.tiledataLo << 8 | renderState.tiledataLo;
      latch.tiledataHi = latch.tiledataHi << 8 | renderState.tiledataHi;

      renderState.sprite = 0;
      for(auto n : range(8)) latch.oam[n] = latch.soam[n];
      renderState.nametable = loadCHR(0x2000 | (uint12)io.v.address);
      step();
      break;

    case 305:
      // tail end of loop
      if(enable() && io.ly == vlines() - 1) {
        io.v.address = io.t.address;
      }
      // do not break, continue into natural case group

    case RENDER_CASE_257_320_1:
      renderState.sprite++;
      renderState.nametable = loadCHR(0x2000 | (uint12)io.v.address);
      step();
      break;

    case 258:
      if(enable()) {
        io.v.nametableX = io.t.nametableX;
        io.v.tileX = io.t.tileX;
      }
      // do not break, continue into natural case group

    case RENDER_CASE_257_320_2:
      step();
      break;

    case RENDER_CASE_257_320_3:
      renderState.attribute = loadCHR(0x23c0 | io.v.nametable << 10 | (io.v.tileY >> 2) << 3 | io.v.tileX >> 2);
      renderState.tileaddr = io.spriteHeight == 8
      ? io.spriteAddress + latch.oam[renderState.sprite].tile * 16
      : (latch.oam[renderState.sprite].tile & ~1) * 16 + (latch.oam[renderState.sprite].tile & 1) * 0x1000;
      step();
      break;
    
    case RENDER_CASE_257_320_4:
      step();
      break;

    case RENDER_CASE_257_320_5:
      renderState.spriteY = (io.ly - latch.oam[renderState.sprite].y) & (io.spriteHeight - 1);
      if(latch.oam[renderState.sprite].attr & 0x80) renderState.spriteY ^= io.spriteHeight - 1;
      renderState.tileaddr += renderState.spriteY + (renderState.spriteY & 8);

      latch.oam[renderState.sprite].tiledataLo = loadCHR(renderState.tileaddr + 0);
      step();
      break;
    
    case RENDER_CASE_257_320_6:
      step();
      break;

    case RENDER_CASE_257_320_7:
      latch.oam[renderState.sprite].tiledataHi = loadCHR(renderState.tileaddr + 8);
      step();
      break;

    case RENDER_CASE_257_320_8:
      step();
      break;

    //321-336 - 8 cases x 2 = 16 
    case RENDER_CASE_321_336_1:
      // tail end of loop
      latch.nametable = latch.nametable << 8 | renderState.nametable;
      latch.attribute = latch.attribute << 2 | (renderState.attribute & 3);
      latch.tiledataLo = latch.tiledataLo << 8 | renderState.tiledataLo;
      latch.tiledataHi = latch.tiledataHi << 8 | renderState.tiledataHi;
      // Do not break, continue to beginning of loop code

    case 321:
      renderState.nametable = loadCHR(0x2000 | (uint12)io.v.address);
      renderState.tileaddr = io.bgAddress | renderState.nametable << 4 | io.v.fineY;
      step();
      break;

    case RENDER_CASE_321_336_2:
      step();
      break;

    case RENDER_CASE_321_336_3:
      renderState.attribute = loadCHR(0x23c0 | io.v.nametable << 10 | (io.v.tileY >> 2) << 3 | io.v.tileX >> 2);
      if(io.v.tileY & 2) renderState.attribute >>= 4;
      if(io.v.tileX & 2) renderState.attribute >>= 2;
      step();
      break;

    case RENDER_CASE_321_336_4:
      if(enable() && ++io.v.tileX == 0) io.v.nametableX++;
      step();
      break;

    case RENDER_CASE_321_336_5:
      renderState.tiledataLo = loadCHR(renderState.tileaddr + 0);
      step();
      break;

    case RENDER_CASE_321_336_6:
      step();
      break;

    case RENDER_CASE_321_336_7:
      renderState.tiledataHi = loadCHR(renderState.tileaddr + 8);
      step();
      break;

    case RENDER_CASE_321_336_8:
      step();
      break;

    //337-338
    case 337:
      // tail end of previous loop (RENDER_CASE_321_336)
      latch.nametable = latch.nametable << 8 | renderState.nametable;
      latch.attribute = latch.attribute << 2 | (renderState.attribute & 3);
      latch.tiledataLo = latch.tiledataLo << 8 | renderState.tiledataLo;
      latch.tiledataHi = latch.tiledataHi << 8 | renderState.tiledataHi;

      loadCHR(0x2000 | (uint12)io.v.address);
      step();
      break;
    
    case 338:
      renderState.skip = enable() && io.field == 1 && io.ly == vlines() - 1;
      step();
      break;
    
    //339
    case 339:
      loadCHR(0x2000 | (uint12)io.v.address);
      step();
      break;

    //340
    case 340:
      if(!renderState.skip) {
        step();
        break;
      }
      // do not break in else case - if skip is false fo directly to next step

    // todo: this run after 340, so 341?
    case 341:
      callcount = 0;
      scanline();
      break;
  }
}
