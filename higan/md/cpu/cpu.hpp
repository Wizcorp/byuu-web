//Motorola 68000

struct CPU : M68K, Thread {
  Node::Component node;
  Node::Instruction eventInstruction;
  Node::Notification eventInterrupt;

  enum class Interrupt : uint {
    Reset,
    HorizontalBlank,
    VerticalBlank,
  };

  // Overclock the CPU
  double overclock = 1.0/7.0;

  //cpu.cpp
  auto load(Node::Object, Node::Object) -> void;
  auto unload() -> void;

  auto main() -> void;
  inline auto step(uint clocks) -> void;
  auto idle(uint clocks) -> void override;
  auto wait(uint clocks) -> void override;

  auto raise(Interrupt) -> void;
  auto lower(Interrupt) -> void;

  auto power(bool reset) -> void;

  //bus.cpp
  auto read(uint1 upper, uint1 lower, uint24 address, uint16 data = 0) -> uint16 override;
  auto write(uint1 upper, uint1 lower, uint24 address, uint16 data) -> void override;

  //io.cpp
  auto readIO(uint1 upper, uint1 lower, uint24 address, uint16 data) -> uint16;
  auto writeIO(uint1 upper, uint1 lower, uint24 address, uint16 data) -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

private:
  Memory::Writable<uint16> ram;
  Memory::Readable<uint16> tmss;
  uint1 tmssEnable;

  struct IO {
    boolean version;  //0 = Model 1; 1 = Model 2+
    boolean romEnable;
    boolean vdpEnable[2];
  } io;

  struct Refresh {
    uint32 ram;
     uint7 external;
  } refresh;

  struct State {
    uint32 interruptLine;
    uint32 interruptPending;
  } state;
};

extern CPU cpu;
