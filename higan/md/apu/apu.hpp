//Zilog Z80

struct APU : Z80, Z80::Bus, Thread {
  Node::Component node;
  Node::Instruction eventInstruction;
  Node::Notification eventInterrupt;

  // Overclock the APU
  double overclock = 1.0/15.0;

  inline auto synchronizing() const -> bool override { return scheduler.synchronizing(); }

  //z80.cpp
  auto load(Node::Object, Node::Object) -> void;
  auto unload() -> void;

  auto main() -> void;
  auto step(uint clocks) -> void override;

  auto power(bool reset) -> void;

  auto running() -> bool { return arbstate.resetLine && busStatus(); }

  alwaysinline auto setNMI(uint1 value) -> void {
    state.nmiLine = value;
  }

  alwaysinline auto setINT(uint1 value) -> void {
    state.intLine = value;
  }

  alwaysinline auto setRES(uint1 value) -> void {
    if(!value && arbstate.resetLine) {
      power(true);
    }
    arbstate.resetLine = value;
  }

  alwaysinline auto setBREQ(uint1 value) -> void {
    arbstate.busreqLine = value;
  }

  auto updateBus() -> void;
  auto busStatus() -> uint1 {
    // 0->68K, 1->Z80
    return (arbstate.resetLine & arbstate.busreqLatch) ^ 1;
  } 

  //bus.cpp
  auto read(uint16 address) -> uint8 override;
  auto write(uint16 address, uint8 data) -> void override;

  auto in(uint16 address) -> uint8 override;
  auto out(uint16 address, uint8 data) -> void override;

  //serialization.cpp
  auto serialize(serializer&) -> void;

private:
  Memory::Writable<uint8> ram;

  struct IO {
    uint9 bank;
  } io;

  struct ArbState {
    uint1 resetLine;
    uint1 busreqLine = 1;
    uint1 busreqLatch = 1;
  } arbstate;

  struct State {
    uint1 nmiLine;
    uint1 intLine;
  } state;
};

extern APU apu;
