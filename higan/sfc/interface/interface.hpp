#if defined(CORE_SFC)

namespace higan::SuperFamicom {

extern Interface* interface;

struct SuperFamicomInterface : Interface {
  auto configure(string name, double value) -> void override;
  
  auto name() -> string override { return "Super Famicom"; }
  auto game() -> string override;

  auto root() -> Node::Object override;
  auto load(Node::Object&, string tree = {}) -> void override;
  auto unload() -> void override;
  auto save() -> void override;
  auto power() -> void override;
  auto run() -> void override;

  auto serialize(bool synchronize = true) -> serializer override;
  auto unserialize(serializer&) -> bool override;

  auto exportMemory() -> bool override;
};

}

#endif
