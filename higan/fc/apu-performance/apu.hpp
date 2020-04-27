#include "Nes_Apu.h"
#include "apu_snapshot.h"

// Byuu APU frequency is 21477273 (NTSC) or 26601713 (PAL)
// blargg APU needs to run at 1789773 clocks/second
#define BLARGG_FREQUENCY 1789773

// Byuu APU sample rate is 1342330 (NTSC) or 2216810 (PAL)
// however this is excessive for blargg APU
#define BLARGG_SAMPLE_RATE 44100

// Size of audio buffer used to get samples from blargg APU
#define BLARGG_BUFFER_SIZE 4096

// Number of clocks we should step in a single tick, this can be any number
// however our audio buffer must be large enough to hold the samples generated. 
#define BLARGG_STEP_SIZE 30

struct APU : Thread {
  Node::Component node;
  Node::Stream stream;

  inline auto rate() const -> uint { return Region::PAL() ? 16 : 12; }

  //apu.cpp
  auto load(Node::Object, Node::Object) -> void;
  auto unload() -> void;

  auto main() -> void;

  auto power(bool reset) -> void;

  auto readIO(uint16 addr) -> uint8;
  auto writeIO(uint16 addr, uint8 data) -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;
  auto serialize_snapshot(serializer&, apu_snapshot_t&) -> void;

private:
  Nes_Apu nes_apu;
  Blip_Buffer blip_buffer;
  int16_t audio_buffer[BLARGG_BUFFER_SIZE];

  blip_time_t frame_clocks;
  auto frameClocks() -> blip_time_t;

  static auto setIRQ(void* _this) -> void;
  static auto readDMC(void* _this, cpu_addr_t addr) -> int;
};

extern APU apu;
