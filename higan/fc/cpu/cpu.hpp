struct CPU : MOS6502, Thread {
  Node::Component node;
  Node::Instruction eventInstruction;
  Node::Notification eventInterrupt;
  Node::Boolean syncOnce;

  inline auto rate() const -> uint { return Region::PAL() ? 16 : 12; }

  // Overclock the CPU
  double overclock = 1.0;

  //cpu.cpp
  auto load(Node::Object, Node::Object) -> void;
  auto unload() -> void;

  auto main() -> void;
  auto step(uint clocks) -> void;

  auto power(bool reset) -> void;

  //memory.cpp
  auto readRAM(uint11 addr) -> uint8;
  auto writeRAM(uint11 addr, uint8 data) -> void;

  auto readIO(uint16 addr) -> uint8;
  auto writeIO(uint16 addr, uint8 data) -> void;

  auto readDebugger(uint16 addr) -> uint8 override;

  auto serialize(serializer&) -> void;

  //timing.cpp
  auto read(uint16 addr) -> uint8 override;
  auto write(uint16 addr, uint8 data) -> void override;
  auto lastCycle() -> void override;
  auto nmi(uint16& vector) -> void override;

  auto oamdma() -> void;

  auto nmiLine(bool) -> void;
  auto irqLine(bool) -> void;
  auto apuLine(bool) -> void;

  auto rdyLine(bool) -> void;
  auto rdyAddr(bool valid, uint16 value = 0) -> void;

//protected:
  uint8 ram[0x800];

  struct IO {
    bool interruptPending = 0;
    bool nmiPending = 0;
    bool nmiLine = 0;
    bool irqLine = 0;
    bool apuLine = 0;

    bool rdyLine = 1;
    bool rdyAddrValid = 0;
    uint16 rdyAddrValue;

    bool oamdmaPending = 0;
    uint8 oamdmaPage;
  } io;
};

extern CPU cpu;
