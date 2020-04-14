struct Emulator {
  struct Firmware;
  
  inline static string GameFolder = "/gamefiles";
  inline static string FirmwaresFolder = "/firmwares";

  static auto construct() -> void;
  auto locate(const string& location, const string& suffix, const string& path = "") -> string;
  auto manifest() -> shared_pointer<vfs::file>;
  auto load(const string& location, const vector<uint8_t>& image) -> bool;
  auto loadFirmware(const Firmware&) -> shared_pointer<vfs::file>;
  auto save() -> void;
  auto unload() -> void;
  auto setBoolean(const string& name, bool value) -> bool;
  auto setOverscan(bool value) -> bool;
  auto error(const string& text) -> void;
  auto errorFirmwareRequired(const Firmware&) -> void;

  virtual auto load() -> bool = 0;
  virtual auto open(higan::Node::Object, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> = 0;

  auto connect(const string& portName, const string& peripheralName) -> bool;
  auto disconnect(const string& portName) -> bool;
  auto setButton(const string& buttonName, int16_t value) -> bool;
  auto setButton(const string& portName, const string& buttonName, int16_t value) -> bool;
  auto input(higan::Node::Input) -> void;

  virtual auto notify(const string& message) -> void {}

  struct Firmware {
    string type;
    string region;
    string sha256;  //optional
    string location;
  };

  struct Game {
    string location;
    string manifest;
    vector<uint8_t> image;
  };

  shared_pointer<higan::Interface> interface;
  string name;
  vector<string> extensions;
  vector<string> ports;
  vector<string> buttons;

  higan::Node::Object root;
  vector<Firmware> firmware;
  Game game;

  private:
    map<string, std::map<string, int16_t>> buttonMaps;
};

extern vector<shared_pointer<Emulator>> emulators;
extern shared_pointer<Emulator> emulator;
