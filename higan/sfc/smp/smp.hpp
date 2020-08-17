//Sony CXP1100Q-1

struct SMP : SPC700, Thread {
  int firstRun = 1;
  
  struct Lockstep {
    bool enabled = true;
    uint cycle = 0;
    uint clocks = 0;
  } lockstep;
  

  Node::Component node;
  Node::Instruction eventInstruction;

  inline auto synchronizing() const -> bool override { return scheduler.synchronizing(); }

  //smp.cpp
  auto load(Node::Object, Node::Object) -> void;
  auto unload() -> void;

  auto main() -> void;
  auto power(bool reset) -> void;

  //io.cpp
  auto portRead(uint2 port) const -> uint8;
  auto portWrite(uint2 port, uint8 data) -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

  uint8 iplrom[64];

private:
  struct IO {
    //timing
    uint clockCounter = 0;
    uint dspCounter = 0;

    //external
    uint8 apu0;
    uint8 apu1;
    uint8 apu2;
    uint8 apu3;

    //$00f0
    uint1 timersDisable;
    uint1 ramWritable = true;
    uint1 ramDisable;
    uint1 timersEnable = true;
    uint2 externalWaitStates;
    uint2 internalWaitStates;

    //$00f1
    uint1 iplromEnable = true;

    //$00f2
    uint8 dspAddress;

    //$00f4-00f7
    uint8 cpu0;
    uint8 cpu1;
    uint8 cpu2;
    uint8 cpu3;

    //$00f8-00f9
    uint8 aux4;
    uint8 aux5;
  } io;

  //memory.cpp
  inline auto readRAM(uint16 address) -> uint8;
  inline auto writeRAM(uint16 address, uint8 data) -> void;

  auto idle() -> void override;
  auto read(uint16 address) -> uint8 override;
  auto write(uint16 address, uint8 data) -> void override;

  auto readDisassembler(uint16 address) -> uint8 override;

  //io.cpp
  inline auto readIO(uint16 address) -> uint8;
  inline auto writeIO(uint16 address, uint8 data) -> void;

  template<uint Frequency>
  struct Timer {
    uint8   stage0;
    uint8   stage1;
    uint8   stage2;
    uint4   stage3;
    boolean line;
    boolean enable;
    uint8   target;

    //timing.cpp
    auto step(uint clocks) -> void;
    auto synchronizeStage1() -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;
  };

  Timer<128> timer0;
  Timer<128> timer1;
  Timer< 16> timer2;

  //timing.cpp
  inline auto wait(bool halve, maybe<uint16> address = nothing) -> void;
  inline auto step(uint clocks) -> void;
  inline auto stepTimers(uint clocks) -> void;
};

extern SMP smp;
