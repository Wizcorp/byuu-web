#include "SPC_DSP.h"

struct DSP : Thread {
  Node::Component node;
  Node::Stream stream;

  uint8 apuram[64 * 1024];

  void main();
  uint8 read(uint8 addr);
  void write(uint8 addr, uint8 data);

  auto load(Node::Object, Node::Object) -> void;
  auto unload() -> void;
  void power(bool reset);
  bool mute();

  void serialize(serializer&);

private:
  SPC_DSP spc_dsp;
  int16_t samplebuffer[8192];
};

extern DSP dsp;
