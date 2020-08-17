struct OAM {
  //oam.cpp
  alwaysinline auto read(uint10 address) -> uint8;
  alwaysinline auto write(uint10 address, uint8 data) -> void;

  //serialization.cpp
  alwaysinline auto serialize(serializer&) -> void;

  struct Object {
    //oam.cpp
    alwaysinline auto width() const -> uint;
    alwaysinline auto height() const -> uint;

    uint9 x;
    uint8 y;
    uint8 character;
    uint1 nameselect;
    uint1 vflip;
    uint1 hflip;
    uint2 priority;
    uint3 palette;
    uint1 size;
  } objects[128];
};
