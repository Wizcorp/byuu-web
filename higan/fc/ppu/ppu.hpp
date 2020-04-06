struct PPU : Thread {
  int callcount = 0;

  // State used in renderScanline
  struct {
    uint attribute;
    uint nametable;
    bool scanlineAfterVBlankCycles;
    bool skip;
    uint sprite; 
    uint spriteY;
    uint tileaddr;
    uint tiledataHi;
    uint tiledataLo;
    uint vBlankCycles;
  } renderState;

  Node::Component node;
  Node::Screen screen;
  Node::String region;
  Node::Boolean colorEmulation;

  inline auto rate() const -> uint { return Region::PAL() ? 5 : 4; }
  inline auto vlines() const -> uint { return Region::PAL() ? 312 : 262; }

  //ppu.cpp
  auto load(Node::Object, Node::Object) -> void;
  auto unload() -> void;

  auto main() -> void;
  #if defined(SCHEDULER_SYNCHRO)
  inline auto step() -> void;
  #else
  inline auto step(uint clocks) -> void;
  #endif
  inline auto scanline() -> void;
  inline auto frame() -> void;
  auto refresh() -> void;

  auto power(bool reset) -> void;

  //memory.cpp
  auto readCIRAM(uint11 addr) -> uint8;
  auto writeCIRAM(uint11 addr, uint8 data) -> void;

  auto readCGRAM(uint5 addr) -> uint8;
  auto writeCGRAM(uint5 addr, uint8 data) -> void;

  auto readIO(uint16 addr) -> uint8;
  auto writeIO(uint16 addr, uint8 data) -> void;

  //render.cpp
  inline auto enable() const -> bool;
  inline auto loadCHR(uint16 addr) -> uint8;

  inline auto renderPixel() -> void;
  inline auto renderSprite() -> void;
  inline auto renderScanline() -> void;

  //color.cpp
  inline auto color(uint32) -> uint64;

  //serialization.cpp
  auto serialize(serializer&) -> void;

  struct IO {
    //internal
    uint8 mdr;

    uint1 field;
    uint lx = 0;
    uint ly = 0;

    uint8 busData;

    struct Union {
      uint19 data;
      BitRange<19, 0, 4> tileX     {&data};
      BitRange<19, 5, 9> tileY     {&data};
      BitRange<19,10,11> nametable {&data};
      BitRange<19,10,10> nametableX{&data};
      BitRange<19,11,11> nametableY{&data};
      BitRange<19,12,14> fineY     {&data};
      BitRange<19, 0,14> address   {&data};
      BitRange<19, 0, 7> addressLo {&data};
      BitRange<19, 8,14> addressHi {&data};
      BitRange<19,15,15> latch     {&data};
      BitRange<19,16,18> fineX     {&data};
    } v, t;

    bool nmiHold = 0;
    bool nmiFlag = 0;

    //$2000
    uint vramIncrement = 1;
    uint spriteAddress = 0;
    uint bgAddress = 0;
    uint spriteHeight = 0;
    bool masterSelect = 0;
    bool nmiEnable = 0;

    //$2001
    bool grayscale = 0;
    bool bgEdgeEnable = 0;
    bool spriteEdgeEnable = 0;
    bool bgEnable = 0;
    bool spriteEnable = 0;
    uint3 emphasis;

    //$2002
    bool spriteOverflow = 0;
    bool spriteZeroHit = 0;

    //$2003
    uint8 oamAddress;
  } io;

  struct OAM {
    //serialization.cpp
    auto serialize(serializer&) -> void;

    uint8 id = 64;
    uint8 y = 0xff;
    uint8 tile = 0xff;
    uint8 attr = 0xff;
    uint8 x = 0xff;

    uint8 tiledataLo = 0;
    uint8 tiledataHi = 0;
  };

  struct Latches {
    uint16 nametable;
    uint16 attribute;
    uint16 tiledataLo;
    uint16 tiledataHi;

    uint oamIterator = 0;
    uint oamCounter = 0;

    OAM oam[8];   //primary
    OAM soam[8];  //secondary
  } latch;

  uint8 ciram[2048];
  uint8 cgram[32];
  uint8 oam[256];

  uint32 buffer[256 * 262];
};

extern PPU ppu;
