#include <fc/fc.hpp>

namespace higan::Famicom {

APU apu;

#include "serialization.cpp"
#include "Blip_Buffer.cpp"
#include "Nes_Apu.cpp"
#include "Nes_Oscs.cpp"
#include "apu_snapshot.cpp"

auto APU::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "APU");
  from = Node::scan(parent = node, from);

  stream = Node::append<Node::Stream>(parent, from, "Stream");
  stream->setChannels(1);
  stream->setFrequency(BLARGG_SAMPLE_RATE);
}

auto APU::unload() -> void {
  node = {};
  stream = {};
}

auto APU::main() -> void {
  // Tick Blargg APU by number of clocks
  nes_apu.end_frame(BLARGG_STEP_SIZE);
	blip_buffer.end_frame(BLARGG_STEP_SIZE);

  // Reset frames clocks as we requested the frame
  frame_clocks = 0;

  // Read any available samples and reset audio buffer
  long count = blip_buffer.read_samples(audio_buffer, BLARGG_BUFFER_SIZE);
  if (count > 0) {
    // Read samples into Byuu audio stream
    for(unsigned n = 0; n < count; n += 1) {
      float sound  = sclamp<16>(audio_buffer[n]) / 32768.0f;
      stream->sample(sound);
    }
  }

  // Step thread by number of clocks
  step(BLARGG_STEP_SIZE);
}

auto APU::setIRQ(void* _this) -> void {
  bool nextIRQ = ((APU*)_this)->nes_apu.earliest_irq();
  cpu.apuLine(nextIRQ == Nes_Apu::irq_waiting);
}

auto APU::readDMC(void* _this, cpu_addr_t addr) -> int {
  return cpu.read(addr);
}

auto APU::power(bool reset) -> void {
  Thread::create(BLARGG_FREQUENCY, {&APU::main, this});

  frame_clocks = 0;

	blip_buffer.clock_rate(BLARGG_FREQUENCY);
	blip_buffer.sample_rate(BLARGG_SAMPLE_RATE);
  blip_buffer.clear();

  nes_apu.reset(Region::PAL());

  nes_apu.output(&blip_buffer);
  nes_apu.osc_output(0, &blip_buffer);
  nes_apu.osc_output(1, &blip_buffer);
  nes_apu.osc_output(2, &blip_buffer);
  nes_apu.osc_output(3, &blip_buffer);
  nes_apu.osc_output(4, &blip_buffer);

  nes_apu.irq_notifier(&APU::setIRQ, this);
  nes_apu.dmc_reader(&APU::readDMC, this);
}

auto APU::frameClocks() -> blip_time_t {
  // NOTE: This should be number of clock cycles relative to the current
  // time frame. It's hard coded for now. However this is called extremely
  // infrequently. Worst case the audio may be a bit less accurate sub-ms.
  // Far from human noticable.
  return frame_clocks += 4;
}

auto APU::readIO(uint16 addr) -> uint8 {
  if (addr == 0x4015) {
    return nes_apu.read_status(frameClocks());
  }

  return cpu.mdr();
}

auto APU::writeIO(uint16 addr, uint8 data) -> void {
  nes_apu.write_register(frameClocks(), addr, data);
}

}
