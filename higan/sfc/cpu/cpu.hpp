struct CPU : WDC65816, Thread, PPUcounter {
  Node::Component node;
  Node::Natural version;
  Node::Instruction eventInstruction;
  Node::Notification eventInterrupt;

  inline auto interruptPending() const -> bool override { return status.interruptPending; }
  inline auto pio() const -> uint8 { return io.pio; }
  inline auto refresh() const -> bool { return status.dramRefresh == 1; }
  inline auto synchronizing() const -> bool override { return scheduler.synchronizing(); }

  // Overclock the CPU
  double overclock = 1.0;

  // See: https://github.com/bsnes-emu/bsnes/blob/8e80d2f8a43e34a82931e25143b279e5fbcfaedc/bsnes/sfc/interface/configuration.cpp#L23
  bool fastMath = true;

  // Set to false to group step executions and execute them once
  // every 4 cycles
  struct Lockstep {
    bool enabled = true;
    uint cycle = 0;
    uint clocks = 0;
  } lockstep;
  
  //cpu.cpp
  auto load(Node::Object parent, Node::Object from) -> void;
  auto unload() -> void;

  auto main() -> void;
  auto map() -> void;
  auto power(bool reset) -> void;

  //dma.cpp
  inline auto dmaEnable() -> bool;
  inline auto hdmaEnable() -> bool;
  inline auto hdmaActive() -> bool;

  auto dmaRun() -> void;
  auto hdmaReset() -> void;
  auto hdmaSetup() -> void;
  auto hdmaRun() -> void;

  //memory.cpp
  auto idle() -> void override;
  auto read(uint24 address) -> uint8 override;
  auto write(uint24 address, uint8 data) -> void override;
  alwaysinline auto wait(uint24 address) const -> uint;
  auto readDisassembler(uint24 address) -> uint8 override;

  //io.cpp
  auto readRAM(uint24 address, uint8 data) -> uint8;
  auto readAPU(uint24 address, uint8 data) -> uint8;
  auto readCPU(uint24 address, uint8 data) -> uint8;
  auto readDMA(uint24 address, uint8 data) -> uint8;
  auto writeRAM(uint24 address, uint8 data) -> void;
  auto writeAPU(uint24 address, uint8 data) -> void;
  auto writeCPU(uint24 address, uint8 data) -> void;
  auto writeDMA(uint24 address, uint8 data) -> void;

  //timing.cpp
  inline auto dmaCounter() const -> uint;
  inline auto joypadCounter() const -> uint;

  auto step(uint clocks) -> void;
  auto scanline() -> void;

  alwaysinline auto aluEdge() -> void;
  alwaysinline auto dmaEdge() -> void;

  //irq.cpp
  alwaysinline auto nmiPoll() -> void;
  alwaysinline auto irqPoll() -> void;
  auto nmitimenUpdate(uint8 data) -> void;
  auto rdnmi() -> bool;
  auto timeup() -> bool;

  alwaysinline auto nmiTest() -> bool;
  alwaysinline auto irqTest() -> bool;
  alwaysinline auto lastCycle() -> void override;

  //joypad.cpp
  auto joypadEdge() -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

  uint8 wram[128 * 1024];
  vector<Thread*> coprocessors;
  vector<Thread*> peripherals;

private:
  struct Counter {
    uint cpu = 0;
    uint dma = 0;
  } counter;

  struct Overclocking {
    uint counter = 0;
    uint target = 0;
  } overclocking;

  struct Status {
    uint clockCount = 0;

    bool irqLock = 0;

    uint dramRefreshPosition = 0;
    uint dramRefresh = 0;  //0 = not refreshed; 1 = refresh active; 2 = refresh inactive

    uint hdmaSetupPosition = 0;
    bool hdmaSetupTriggered = 0;

    uint hdmaPosition = 0;
    bool hdmaTriggered = 0;

    boolean nmiValid;
    boolean nmiLine;
    boolean nmiTransition;
    boolean nmiPending;
    boolean nmiHold;

    boolean irqValid;
    boolean irqLine;
    boolean irqTransition;
    boolean irqPending;
    boolean irqHold;

    bool resetPending = 0;
    bool interruptPending = 0;

    bool dmaActive = 0;
    bool dmaPending = 0;
    bool hdmaPending = 0;
    bool hdmaMode = 0;  //0 = init, 1 = run

    bool autoJoypadActive = 0;
    bool autoJoypadLatch = 0;
    uint autoJoypadCounter = 0;
  } status;

  struct IO {
    //$2181-$2183
    uint17 wramAddress;

    //$4200
    boolean hirqEnable;
    boolean virqEnable;
    boolean irqEnable;
    boolean nmiEnable;
    boolean autoJoypadPoll;

    //$4201
    uint8 pio = 0xff;

    //$4202-$4203
    uint8 wrmpya = 0xff;
    uint8 wrmpyb = 0xff;

    //$4204-$4206
    uint16 wrdiva = 0xffff;
    uint8 wrdivb = 0xff;

    //$4207-$420a
    uint12 htime = 0x1ff + 1 << 2;
    uint9  vtime = 0x1ff;

    //$420d
    uint romSpeed = 8;

    //$4210
    uint4 version = 2;  //allowed values: 1, 2

    //$4214-$4217
    uint16 rddiv;
    uint16 rdmpy;

    //$4218-$421f
    uint16 joy1;
    uint16 joy2;
    uint16 joy3;
    uint16 joy4;
  } io;

  struct ALU {
    uint mpyctr = 0;
    uint divctr = 0;
    uint shift = 0;
  } alu;

  struct Channel {
    //dma.cpp
    inline auto step(uint clocks) -> void;
    inline auto edge() -> void;

    inline auto validA(uint24 address) -> bool;
    inline auto readA(uint24 address) -> uint8;
    inline auto readB(uint8 address, bool valid) -> uint8;
    inline auto writeA(uint24 address, uint8 data) -> void;
    inline auto writeB(uint8 address, uint8 data, bool valid) -> void;
    inline auto transfer(uint24 address, uint2 index) -> void;

    inline auto dmaRun() -> void;
    inline auto hdmaActive() -> bool;
    inline auto hdmaFinished() -> bool;
    inline auto hdmaReset() -> void;
    inline auto hdmaSetup() -> void;
    inline auto hdmaReload() -> void;
    inline auto hdmaTransfer() -> void;
    inline auto hdmaAdvance() -> void;

    //$420b
    uint1 dmaEnable;

    //$420c
    uint1 hdmaEnable;

    //$43x0
    uint3 transferMode = 7;
    uint1 fixedTransfer = 1;
    uint1 reverseTransfer = 1;
    uint1 unused = 1;
    uint1 indirect = 1;
    uint1 direction = 1;

    //$43x1
    uint8 targetAddress = 0xff;

    //$43x2-$43x3
    uint16 sourceAddress = 0xffff;

    //$43x4
    uint8 sourceBank = 0xff;

    //$43x5-$43x6
    union {
      uint16 transferSize;
      uint16 indirectAddress;
    };

    //$43x7
    uint8 indirectBank = 0xff;

    //$43x8-$43x9
    uint16 hdmaAddress = 0xffff;

    //$43xa
    uint8 lineCounter = 0xff;

    //$43xb/$43xf
    uint8 unknown = 0xff;

    //internal state
    uint1 hdmaCompleted;
    uint1 hdmaDoTransfer;

    maybe<Channel&> next;

    Channel() : transferSize(0xffff) {}
  } channels[8];
};

extern CPU cpu;
