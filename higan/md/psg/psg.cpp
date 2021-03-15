#include <md/md.hpp>

namespace higan::MegaDrive {

PSG psg;
#include "serialization.cpp"

auto PSG::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "PSG");
  from = Node::scan(parent = node, from);

  stream = Node::append<Node::Stream>(parent, from, "Stream");
  stream->setChannels(1);
  stream->setFrequency(system.frequency() / 15.0 / 16.0);
  stream->addHighPassFilter(  20.0, 1);
  stream->addLowPassFilter (2840.0, 1);
}

auto PSG::unload() -> void {
  node = {};
  stream = {};
}

auto PSG::main() -> void {
  auto channels = SN76489::clock();
  float output = 0.0;
  output += volume[channels[0]];
  output += volume[channels[1]];
  output += volume[channels[2]];
  output += volume[channels[3]];
  stream->sample(output / 4.0f * 0.625f);
  step(16);
}

auto PSG::step(uint clocks) -> void {
  Thread::step(clocks);
#if !defined(SCHEDULER_SYNCHRO)
  Thread::synchronize(cpu, apu);
#endif
}

auto PSG::power(bool reset) -> void {
  SN76489::power();
  Thread::create(system.frequency() / 15.0, {&PSG::main, this});

  for(uint level : range(15)) {
    volume[level] = pow(2, level * -2.0 / 6.0);
  }
  volume[15] = 0;
}

}
