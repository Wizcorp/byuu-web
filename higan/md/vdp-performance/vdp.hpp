//Yamaha YM7101
struct VDP : Thread {
  Node::Component node;
  Node::Screen screen;
  Node::String region;

  bool hasRendered = false;
  bool optimizeSteps = false;
  bool isSkipping = false;
  bool skip;

  //vdp.cpp
  auto load(Node::Object, Node::Object) -> void;
  auto unload() -> void;

  auto main() -> void;
  auto step(uint clocks) -> void;
  auto refresh() -> void;
  auto render() -> void;
  auto power(bool reset) -> void;


  auto readDataPort() -> uint16 {
    io.commandPending = false;

    //VRAM read
    if(io.command.bit(0,3) == 0) {
      auto address = io.address.bit(1,16);
      auto data = vram.read(address);
      io.address += io.dataIncrement;
      return data;
    }

    //VSRAM read
    if(io.command.bit(0,3) == 4) {
      auto address = io.address.bit(1,6);
      auto data = vsram.read(address);
      io.address += io.dataIncrement;
      return data;
    }

    //CRAM read
    if(io.command.bit(0,3) == 8) {
      auto address = io.address.bit(1,6);
      auto data = cram.read(address);
      io.address += io.dataIncrement;
      return data.bit(0,2) << 1 | data.bit(3,5) << 5 | data.bit(6,8) << 9;
    }

    return 0x0000;
  }

  auto writeDataPort(uint16 data) -> void {
    io.commandPending = false;

    //DMA VRAM fill
    if(dma.io.wait) {
      dma.io.wait = false;
      dma.io.fill = data >> 8;
      //falls through to memory write
      //causes extra transfer to occur on VRAM fill operations
    }

    //VRAM write
    if(io.command.bit(0,3) == 1) {
      auto address = io.address.bit(1,16);
      if(io.address.bit(0)) data = data >> 8 | data << 8;
      vram.write(address, data);
      io.address += io.dataIncrement;
      return;
    }

    //VSRAM write
    if(io.command.bit(0,3) == 5) {
      auto address = io.address.bit(1,6);
      //data format: ---- --yy yyyy yyyy
      vsram.write(address, data.bit(0,9));
      io.address += io.dataIncrement;
      return;
    }

    //CRAM write
    if(io.command.bit(0,3) == 3) {
      auto address = io.address.bit(1,6);
      //data format: ---- bbb- ggg- rrr-
      cram.write(address, data.bit(1,3) << 0 | data.bit(5,7) << 3 | data.bit(9,11) << 6);
      io.address += io.dataIncrement;
      return;
    }
  }

  //

  auto readControlPort() -> uint16 {
    io.commandPending = false;

    uint16 result;
    result.bit( 0) = Region::PAL();
    result.bit( 1) = io.command.bit(5);  //DMA active
    result.bit( 2) = state.hcounter >= 1280;  //horizontal blank
    result.bit( 3) = state.vcounter >= screenHeight();  //vertical blank
    result.bit( 4) = io.interlaceMode.bit(0) && state.field;
    result.bit( 5) = 0;  //SCOL
    result.bit( 6) = 0;  //SOVR
    result.bit( 7) = io.vblankIRQ;
    result.bit( 8) = 0;  //FIFO full
    result.bit( 9) = 1;  //FIFO empty
    result.bit(10) = 1;  //constants (bits 10-15)
    result.bit(11) = 0;
    result.bit(12) = 1;
    result.bit(13) = 1;
    result.bit(14) = 0;
    result.bit(15) = 0;
    return result;
  }

  auto writeControlPort(uint16 data) -> void {
    //command write (lo)
    if(io.commandPending) {
      io.commandPending = false;

      io.command.bit(2,5) = data.bit(4,7);
      io.address.bit(14,16) = data.bit(0,2);

      if(!dma.io.enable) io.command.bit(5) = 0;
      if(dma.io.mode == 3) dma.io.wait = false;
      return;
    }

    //command write (hi)
    if(data.bit(14,15) != 2) {
      io.commandPending = true;

      io.command.bit(0,1) = data.bit(14,15);
      io.address.bit(0,13) = data.bit(0,13);
      return;
    }

    //register write (d13 is ignored)
    if(data.bit(14,15) == 2)
    switch(data.bit(8,12)) {

    //mode register 1
    case 0x00: {
      io.displayOverlayEnable = data.bit(0);
      io.counterLatch = data.bit(1);
      io.horizontalBlankInterruptEnable = data.bit(4);
      io.leftColumnBlank = data.bit(5);
      return;
    }

    //mode register 2
    case 0x01: {
      io.videoMode = data.bit(2);
      io.overscan = data.bit(3);
      dma.io.enable = data.bit(4);
      io.verticalBlankInterruptEnable = data.bit(5);
      io.displayEnable = data.bit(6);
      vram.mode = data.bit(7);
      if(!dma.io.enable) io.command.bit(5) = 0;
      return;
    }

    //plane A name table location
    case 0x02: {
      planeA.io.nametableAddress.bit(12,15) = data.bit(3,6);
      return;
    }

    //window name table location
    case 0x03: {
      window.io.nametableAddress.bit(10,15) = data.bit(1,6);
      return;
    }

    //plane B name table location
    case 0x04: {
      planeB.io.nametableAddress.bit(12,15) = data.bit(0,3);
      return;
    }

    //sprite attribute table location
    case 0x05: {
      sprite.io.nametableAddress.bit(8,15) = data.bit(0,7);
      return;
    }

    //sprite pattern base address
    case 0x06: {
      sprite.io.generatorAddress.bit(15) = data.bit(5);
      return;
    }

    //background color
    case 0x07: {
      io.backgroundColor = data.bit(4,5) << 0 | data.bit(0,3) << 3;
      return;
    }

    //horizontal interrupt counter
    case 0x0a: {
      io.horizontalInterruptCounter = data.bit(0,7);
      return;
    }

    //mode register 3
    case 0x0b: {
      planeA.io.horizontalScrollMode = data.bit(0,1);
      planeB.io.horizontalScrollMode = data.bit(0,1);
      planeA.io.verticalScrollMode = data.bit(2);
      planeB.io.verticalScrollMode = data.bit(2);
      io.externalInterruptEnable = data.bit(3);
      return;
    }

    //mode register 4
    case 0x0c: {
      io.displayWidth = data.bit(0) | data.bit(7) << 1;
      io.interlaceMode = data.bit(1,2);
      io.shadowHighlightEnable = data.bit(3);
      io.externalColorEnable = data.bit(4);
      io.horizontalSync = data.bit(5);
      io.verticalSync = data.bit(6);
      return;
    }

    //horizontal scroll data location
    case 0x0d: {
      planeA.io.horizontalScrollAddress = data.bit(0,6) << 9;
      planeB.io.horizontalScrollAddress = data.bit(0,6) << 9;
      return;
    }

    //nametable pattern base address
    case 0x0e: {
      //bit(0) relocates plane A to the extended VRAM region.
      //bit(4) relocates plane B, but only when bit(0) is also set.
      planeA.io.generatorAddress.bit(15) = data.bit(0);
      planeB.io.generatorAddress.bit(15) = data.bit(4) && data.bit(0);
      return;
    }

    //data port auto-increment value
    case 0x0f: {
      io.dataIncrement = data.bit(0,7);
      return;
    }

    //plane size
    case 0x10: {
      planeA.io.nametableWidth = data.bit(0,1);
      planeB.io.nametableWidth = data.bit(0,1);
      planeA.io.nametableHeight = data.bit(4,5);
      planeB.io.nametableHeight = data.bit(4,5);
      return;
    }

    //window plane horizontal position
    case 0x11: {
      window.io.horizontalOffset = data.bit(0,4) << 4;
      window.io.horizontalDirection = data.bit(7);
      return;
    }

    //window plane vertical position
    case 0x12: {
      window.io.verticalOffset = data.bit(0,4) << 3;
      window.io.verticalDirection = data.bit(7);
      return;
    }

    //DMA length
    case 0x13: {
      dma.io.length.bit(0,7) = data.bit(0,7);
      return;
    }

    //DMA length
    case 0x14: {
      dma.io.length.bit(8,15) = data.bit(0,7);
      return;
    }

    //DMA source
    case 0x15: {
      dma.io.source.bit(0,7) = data.bit(0,7);
      return;
    }

    //DMA source
    case 0x16: {
      dma.io.source.bit(8,15) = data.bit(0,7);
      return;
    }

    //DMA source
    case 0x17: {
      dma.io.source.bit(16,21) = data.bit(0,5);
      dma.io.mode = data.bit(6,7);
      dma.io.wait = dma.io.mode.bit(1);
      return;
    }

    //unused
    default: {
      return;
    }

    }
  }


  //io.cpp
  inline auto read(uint24 address, uint16) -> uint16 {
    switch(address & 0xc0001e) {

    //data port
    case 0xc00000: case 0xc00002: {
      return readDataPort();
    }

    //control port
    case 0xc00004: case 0xc00006: {
      return readControlPort();
    }

    //counter
    case 0xc00008: case 0xc0000a: case 0xc0000c: case 0xc0000e: {
      auto vcounter = state.vcounter;
      if(io.interlaceMode.bit(0)) {
        if(io.interlaceMode.bit(1)) vcounter <<= 1;
        vcounter.bit(0) = vcounter.bit(8);
      }
      return vcounter << 8 | (state.hdot >> 1) << 0;
    }

    }

    return 0x0000;
  }

  inline auto write(uint24 address, uint16 data) -> void {
    switch(address & 0xc0001e) {

    //data port
    case 0xc00000: case 0xc00002: {
      return writeDataPort(data);
    }

    //control port
    case 0xc00004: case 0xc00006: {
      return writeControlPort(data);
    }

    }
  }

  //color.cpp
  auto color(uint32) -> uint64;

  //serialization.cpp
  auto serialize(serializer&) -> void;

private:
  auto pixelWidth() const -> uint { return latch.displayWidth ? 4 : 5; }
  auto screenWidth() const -> uint { return latch.displayWidth ? 320 : 256; }
  auto screenHeight() const -> uint { return latch.overscan ? 240 : 224; }
  auto frameHeight() const -> uint { return Region::PAL() ? 312 : 262; }

  uint32 buffer[320 * 512];
  uint32* output = nullptr;

  struct VRAM {
    //memory.cpp
    auto read(uint16 address) const -> uint16;
    auto write(uint16 address, uint16 data) -> void;

    auto readByte(uint17 address) const -> uint8;
    auto writeByte(uint17 address, uint8 data) -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;

     uint8 pixels[131072];
    uint16 memory[65536];
    uint32 size = 32768;
     uint1 mode;  //0 = 64KB, 1 = 128KB
  } vram;

  struct VSRAM {
    //memory.cpp
    auto read(uint6 address) const -> uint10;
    auto write(uint6 address, uint10 data) -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;

    uint10 memory[40];
  } vsram;

  struct CRAM {
    //memory.cpp
    auto read(uint6 address) const -> uint9;
    auto write(uint6 address, uint9 data) -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;

     uint9 memory[64];
    uint32 palette[3 * 128];
  } cram;

  struct DMA {
    //dma.cpp
    alwaysinline auto run() -> bool;
    alwaysinline auto load() -> void;
    alwaysinline auto fill() -> void;
    alwaysinline auto copy() -> void;

    alwaysinline auto power() -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;

    uint1 active;

    struct IO {
       uint2 mode;
      uint22 source;
      uint16 length;
       uint8 fill;
       uint1 enable;
       uint1 wait;
    } io;
  } dma;

  struct Background {
    enum class ID : uint { PlaneA, Window, PlaneB } id;

    //background.cpp
    auto renderScreen(uint from, uint to) -> void;
    auto renderWindow(uint from, uint to) -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;

    struct IO {
      uint16 generatorAddress;
      uint16 nametableAddress;

      //PlaneA, PlaneB
       uint2 nametableWidth;
       uint2 nametableHeight;
      uint15 horizontalScrollAddress;
       uint2 horizontalScrollMode;
       uint1 verticalScrollMode;

      //Window
      uint10 horizontalOffset;
       uint1 horizontalDirection;
      uint10 verticalOffset;
       uint1 verticalDirection;
    } io;

  //unserialized:
    uint7 pixels[320];
  };
  Background planeA{Background::ID::PlaneA};
  Background window{Background::ID::Window};
  Background planeB{Background::ID::PlaneB};

  struct Object {
    //object.cpp
    inline auto width() const -> uint;
    inline auto height() const -> uint;

    //serialization.cpp
    auto serialize(serializer&) -> void;

     uint9 x;
    uint10 y;
     uint2 tileWidth;
     uint2 tileHeight;
     uint1 horizontalFlip;
     uint1 verticalFlip;
     uint2 palette;
     uint1 priority;
    uint11 address;
     uint7 link;
  };

  struct Sprite {
    //sprite.cpp
    auto render() -> void;
    auto write(uint9 address, uint16 data) -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;

    struct IO {
      uint16 generatorAddress;
      uint16 nametableAddress;
    } io;

    Object oam[80];
    Object objects[20];

  //unserialized:
    uint7 pixels[512];
  } sprite;

  struct State {
    uint16 hdot;
    uint16 hcounter;
    uint16 vcounter;
     uint1 field;
  } state;

  struct IO {
    //status
     uint1 vblankIRQ;  //true after VIRQ triggers; cleared at start of next frame

    //command
     uint6 command;
    uint17 address;
     uint1 commandPending;

    //$00  mode register 1
     uint1 displayOverlayEnable;
     uint1 counterLatch;
     uint1 horizontalBlankInterruptEnable;
     uint1 leftColumnBlank;

    //$01  mode register 2
     uint1 videoMode;  //0 = Master System; 1 = Mega Drive
     uint1 overscan;   //0 = 224 lines; 1 = 240 lines
     uint1 verticalBlankInterruptEnable;
     uint1 displayEnable;

    //$07  background color
     uint7 backgroundColor;

    //$0a  horizontal interrupt counter
     uint8 horizontalInterruptCounter;

    //$0b  mode register 3
     uint1 externalInterruptEnable;

    //$0c  mode register 4
     uint2 displayWidth;
     uint2 interlaceMode;
     uint1 shadowHighlightEnable;
     uint1 externalColorEnable;
     uint1 horizontalSync;
     uint1 verticalSync;

    //$0f  data port auto-increment value
     uint8 dataIncrement;
  } io;

  struct Latch {
    //per-frame
     uint1 interlace;
     uint1 overscan;
     uint8 horizontalInterruptCounter;

    //per-scanline
     uint2 displayWidth;
  } latch;

//unserialized:
  uint8 lookupBG[1 << 10];
  uint8 lookupFG[1 << 15];

  friend class CPU;
  friend class APU;
};

extern VDP vdp;
