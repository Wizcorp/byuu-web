#include <sfc/sfc.hpp>

#define DSP_CPP
namespace higan::SuperFamicom {

DSP dsp;

#include "serialization.cpp"
#include "SPC_DSP.cpp"

auto DSP::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "DSP");
  from = Node::scan(parent = node, from);

  stream = Node::append<Node::Stream>(parent, from, "Stream");
  stream->setChannels(2);
  stream->setFrequency(system.apuFrequency() / 768.0);
}

auto DSP::unload() -> void {
  node = {};
  stream = {};
}

void DSP::main() {
  // Tick Blargg DSP by number if clocks
  // NOTE: we use 32 to improve performance by instantly generating a sample
  // This will then go quiet until the next needed sample, however this will
  // use up a bit more memory. As long as the thread frequency is set properly
  // this will properly sync up with other subsystem threads regardless of how
  // many ticks we jump
  uintmax clocks = 32;
  
  if(enabled) spc_dsp.run(clocks);

  step(clocks);
  
  if(!enabled) return;

  // The above will generate a sample every 32 ticks, if there is a sample we
  // need to pass it onto the client for playback
  signed count = spc_dsp.sample_count();
  if(count > 0) {
    for(unsigned n = 0; n < count; n += 2) {
      float left  = samplebuffer[n + 0] / 32768.0f;
      float right = samplebuffer[n + 1] / 32768.0f;
      stream->sample(left, right);
    }
    spc_dsp.set_output(samplebuffer, 8192);
  }
}

uint8 DSP::read(uint8 addr) {
  return spc_dsp.read(addr);
}

void DSP::write(uint8 addr, uint8 data) {
  spc_dsp.write(addr, data);
}

void DSP::power(bool reset) {
  // Byuu APU frequency is 24606720/second
  // However blargg requires 1024000/second
  #define BLARGG_FREQUENCY 1024000
  Thread::create(BLARGG_FREQUENCY, {&DSP::main, this});

  if (!reset) {
    spc_dsp.init(apuram);
    spc_dsp.reset();
    spc_dsp.set_output(samplebuffer, 8192);
  } else {
    spc_dsp.soft_reset();
    spc_dsp.set_output(samplebuffer, 8192);
  }
}

bool DSP::mute() {
  return spc_dsp.mute();
}

}
