struct VDP : TMS9918, Thread {
  Node::Component node;
  Node::Screen screen;

  //vdp.cpp
  auto load(Node::Object, Node::Object) -> void;
  auto unload() -> void;
  auto main() -> void;

  auto step(uint clocks) -> void override;
  auto irq(bool line) -> void override;
  auto frame() -> void override;
  auto refresh() -> void;
  auto power() -> void;

  //color.cpp
  auto color(uint32) -> uint64;

  //serialization.cpp
  auto serialize(serializer&) -> void;
};

extern VDP vdp;
