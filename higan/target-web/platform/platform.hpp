struct WebPlatform;

struct LoadingCallbackContainer {
    LoadingCallbackContainer(
        WebPlatform *i, 
        const char *u, 
        emscripten::val f,
        emscripten::val c
    ) : files(f), callback(c) {
        url = u;
        instance = i;
    }; 
    WebPlatform *instance;
    string url;
    emscripten::val files;
    emscripten::val callback;
};

using Object = higan::Node::Object;
using Screen = higan::Node::Screen;
using Stream =higan::Node::Stream;
using Input = higan::Node::Input;
using Event = higan::Event;

struct WebPlatform : higan::Platform {
    static struct Error {
        static constexpr const char *MATCHING_EMULATOR_NOT_FOUND     = "ERROR_WEBPLATFORM_MATCHING_EMULATOR_NOT_FOUND";
        static constexpr const char *ROM_FETCH_FAILED                = "ERROR_WEBPLATFORM_ROM_FETCH_FAILED";
        static constexpr const char *EMULATOR_ROM_LOAD_FAILED        = "ERROR_WEBPLATFORM_EMULATOR_ROM_LOAD_FAILED";
    } Error;

    public:
        higan::Node::Screen screen;

        // Is the emulator started?
        bool started = false;

        // Is a cycle running?
        bool running = false;

        nall::shared_pointer<Emulator> emulator;
        // vector<higan::Node::Screen> screens; // todo: do we need to track screens?
        vector<higan::Node::Stream> streams;

        // Frame event callbacks
        emscripten::val onFrameStart = emscripten::val::null();
        emscripten::val onFrameEnd = emscripten::val::null();

        WebPlatform();

        auto initialize(const char *windowTitle) -> bool;
        auto terminate() -> void;

        auto onResize(emscripten::val callback) -> void;

        auto getROMInfo(const char *path, uint8_t *rom, int size) -> emscripten::val;
        
        auto getEmulatorForFilename(const char *path) -> string;
        auto setEmulator(const char *emulatorName) -> bool;
        auto setEmulatorForFilename(const char *path) -> bool;
        
        auto load(uint8_t *rom, int size, emscripten::val files) -> emscripten::val;
        auto unload() -> void;

        auto run() -> void;
        auto configure(string name, uint value) -> void;
        auto setVolume(uint volume) -> void;
        auto setMute(bool mute) -> void;

        auto attach(Object object) -> void override;
        auto detach(Object object) -> void override;
        auto open(
            Object node, 
            string name, 
            vfs::file::mode mode, 
            bool required = false
        ) -> shared_pointer<vfs::file> override;
        
        auto log(string_view message) -> void override;
        
        auto video(Screen, const uint32_t* data, uint pitch, uint width, uint height) -> void override;
        auto audio(Stream stream) -> void override;

        auto connect(const string& portName, const string& peripheralName) -> bool;
        auto disconnect(const string& portName) -> bool;
        auto setButton(const string& portName, const string& buttonName, int16_t value) -> bool;
        auto input(Input node) -> void override;

        auto stateSave(emscripten::val callback) -> void;
        auto stateLoad(const char *stateData, uint stateSize) -> bool;
        auto save() -> emscripten::val;
    
        auto getEmulatorAndGameInfo(nall::shared_pointer<Emulator> emulator, string manifest) -> emscripten::val;

    private:
        WebVideo webvideo;
        WebAudio webaudio;

        auto createJSObjectFromManifest(string& manifest) -> emscripten::val;
        auto parseBMLNode(Markup::Node node) -> emscripten::val;
        auto getEmulatorByName(const char *name) -> nall::shared_pointer<Emulator>;
        auto getEmulatorByExtension(const char *extension) -> nall::shared_pointer<Emulator>;
        auto getFilenameExtension(const char *url) -> const char*;
};