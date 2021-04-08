#if defined(PROFILE_PERFORMANCE)
#include "../dsp-performance/dsp.cpp"
#else
#include <sfc/sfc.hpp>

namespace higan::SuperFamicom {

DSP dsp;
#include "memory.cpp"
#include "gaussian.cpp"
#include "counter.cpp"
#include "envelope.cpp"
#include "brr.cpp"
#include "misc.cpp"
#include "voice.cpp"
#include "echo.cpp"
#include "serialization.cpp"

auto DSP::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Component>(parent, from, "DSP");
  from = Node::scan(parent = node, from);

  stream = Node::append<Node::Stream>(parent, from, "Stream");
  stream->setChannels(2);
  stream->setFrequency(system.apuFrequency() * overclock);
}

auto DSP::unload() -> void {
  node = {};
  stream = {};
}

auto DSP::main() -> void {
  if (enabled) {
    switch(callcount) {
      case 0:
        voice5(voice[0]);
        voice2(voice[1]);
        break;
      case 1:
        voice6(voice[0]);
        voice3(voice[1]);
        break;
      case 2: 
        voice7(voice[0]);
        voice4(voice[1]);
        voice1(voice[3]);
        break;
      case 3:
        voice8(voice[0]);
        voice5(voice[1]);
        voice2(voice[2]);
        break;
      case 4:
        voice9(voice[0]);
        voice6(voice[1]);
        voice3(voice[2]);
        break;
      case 5:
        voice7(voice[1]);
        voice4(voice[2]);
        voice1(voice[4]);
        break;
      case 6: 
        voice8(voice[1]);
        voice5(voice[2]);
        voice2(voice[3]);
        break;
      case 7:
        voice9(voice[1]);
        voice6(voice[2]);
        voice3(voice[3]);
        break;
      case 8:
        voice7(voice[2]);
        voice4(voice[3]);
        voice1(voice[5]);
        break;
      case 9:
        voice8(voice[2]);
        voice5(voice[3]);
        voice2(voice[4]);
        break;
      case 10:
        voice9(voice[2]);
        voice6(voice[3]);
        voice3(voice[4]);
        break;
      case 11:
        voice7(voice[3]);
        voice4(voice[4]);
        voice1(voice[6]);
        break;
      case 12:
        voice8(voice[3]);
        voice5(voice[4]);
        voice2(voice[5]);
        break;
      case 13:
        voice9(voice[3]);
        voice6(voice[4]);
        voice3(voice[5]);
        break;
      case 14:
        voice7(voice[4]);
        voice4(voice[5]);
        voice1(voice[7]);
        break;
      case 15:
        voice8(voice[4]);
        voice5(voice[5]);
        voice2(voice[6]);
        break;
      case 16:
        voice9(voice[4]);
        voice6(voice[5]);
        voice3(voice[6]);
        break;
      case 17:
        voice1(voice[0]);
        voice7(voice[5]);
        voice4(voice[6]);
        break;
      case 18:
        voice8(voice[5]);
        voice5(voice[6]);
        voice2(voice[7]);
        break;
      case 19:
        voice9(voice[5]);
        voice6(voice[6]);
        voice3(voice[7]);
        break;
      case 20:
        voice1(voice[1]);
        voice7(voice[6]);
        voice4(voice[7]);
        break;
      case 21:
        voice8(voice[6]);
        voice5(voice[7]);
        voice2(voice[0]);
        break;
      case 22:
        voice3a(voice[0]);
        voice9(voice[6]);
        voice6(voice[7]);
        echo22();
        break;
      case 23:
        voice7(voice[7]);
        echo23();
        break;
      case 24:
        voice8(voice[7]);
        echo24();
        break;
      case 25:
        voice3b(voice[0]);
        voice9(voice[7]);
        echo25();
        break;
      case 26:
        echo26();
        break;
      case 27:
        misc27();
        echo27();
        break;
      case 28:
        misc28();
        echo28();
        break;
      case 29:
        misc29();
        echo29();
        break;
      case 30:
        misc30();
        voice3c(voice[0]);
        echo30();
        break;
      case 31:
        voice4(voice[0]);
        voice1(voice[2]);
        break;
    }
  }

  callcount = (callcount + 1) % 32;
  tick();
}

auto DSP::tick() -> void {
  Thread::step(3 * 8);
  Thread::synchronize(smp);
}

auto DSP::sample(int16 left, int16 right) -> void {
  stream->sample(left / 32768.0f, right / 32768.0f);
}

auto DSP::power(bool reset) -> void {
  Thread::create(system.apuFrequency(), {&DSP::main, this});

  if(!reset) {
    random.array(apuram, sizeof(apuram));
    random.array(registers, sizeof(registers));
  }

  master = {};
  echo = {};
  noise = {};
  brr = {};
  latch = {};
  for(uint n : range(8)) {
    voice[n] = {};
    voice[n].index = n << 4;
  }
}

}
#endif