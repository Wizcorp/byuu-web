#pragma once

namespace higan {

struct Interface {
  virtual ~Interface() = default;

  //configuration
  virtual auto configure(string name, double value) -> void {}
  
  //information
  virtual auto name() -> string { return {}; }
  virtual auto game() -> string { return {}; }

  //system interface
  virtual auto root() -> Node::Object { return {}; }
  virtual auto load(Node::Object&, string tree = {}) -> void {}
  virtual auto power() -> void {}
  virtual auto run() -> void {}
  virtual auto save() -> void {}
  virtual auto unload() -> void {}

  //state functions
  virtual auto serialize(bool synchronize = true) -> serializer { return {}; }
  virtual auto unserialize(serializer&) -> bool { return false; }

  //debugging functions
  virtual auto exportMemory() -> bool { return false; }
};

}
