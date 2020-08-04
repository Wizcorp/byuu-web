struct AbstractMemory {
  virtual ~AbstractMemory() { reset(); }
  inline explicit operator bool() const { return size() > 0; }

  virtual auto reset() -> void {}
  virtual auto allocate(uint, uint8 = 0xff) -> void {}

  virtual auto load(shared_pointer<vfs::file> fp) -> void {}
  virtual auto save(shared_pointer<vfs::file> fp) -> void {}

  virtual auto data() -> uint8* = 0;
  virtual auto size() const -> uint = 0;

  virtual auto read(uint24 address, uint8 data = 0) -> uint8 = 0;
  virtual auto write(uint24 address, uint8 data) -> void = 0;

  uint id = 0;
};

#include "readable.hpp"
#include "writable.hpp"
#include "protectable.hpp"

struct Bus {
  enum fastmode_t { FastModeNone, FastModeReadOnly, FastModeReadWrite } fastmode;

  alwaysinline static auto mirror(uint address, uint size) -> uint;
  alwaysinline static auto reduce(uint address, uint mask) -> uint;

  ~Bus();

  alwaysinline auto read(uint24 address, uint8 data) -> uint8;
  alwaysinline auto write(uint24 address, uint8 data) -> void;

  auto reset() -> void;
  auto map(
    const function<uint8 (uint24, uint8)>& read,
    const function<void  (uint24, uint8)>& write,
    const string& address, uint size = 0, uint base = 0, uint mask = 0,
    unsigned fastmode = 0, uint8* fast_ptr = NULL
  ) -> uint;
  auto unmap(const string& address) -> void;

private:
  uint8 lookup[16 * 1024 * 1024];
  uint32 target[16 * 1024 * 1024];

  static const unsigned fast_page_size_bits = 13;//keep at 13 or lower so the RAM mirrors can be on the fast path
  static const unsigned fast_page_size = (1 << fast_page_size_bits);
  static const unsigned fast_page_size_mask = (fast_page_size - 1);
  uint8* fast_read[0x1000000>>fast_page_size_bits];
  uint8* fast_write[0x1000000>>fast_page_size_bits];

  function<uint8 (uint24, uint8)> reader[256];
  function<void  (uint24, uint8)> writer[256];
  uint24 counter[256];
};

extern Bus bus;
