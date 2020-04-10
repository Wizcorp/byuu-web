struct WebPlatform;

struct LoadingCallbackContainer {
    LoadingCallbackContainer(WebPlatform *i, const char *u, emscripten::val c) : callback(c) {
        url = u;
        instance = i;
    }; 
    WebPlatform *instance;
    const char *url;
    emscripten::val callback;
};

using higan::Node::Object;
using higan::Node::Screen;
using higan::Node::Stream;
using higan::Node::Input;
using higan::Event;

struct WebPlatform : higan::Platform {
    static struct Error {
        static constexpr const char *MATCHING_EMULATOR_NOT_FOUND     = "ERROR_WEBPLATFORM_MATCHING_EMULATOR_NOT_FOUND";
        static constexpr const char *ROM_FETCH_FAILED                = "ERROR_WEBPLATFORM_ROM_FETCH_FAILED";
        static constexpr const char *EMULATOR_ROM_LOAD_FAILED        = "ERROR_WEBPLATFORM_EMULATOR_ROM_LOAD_FAILED";
    } Error;

    public:
        // Is the emulator started?
        bool started = false;

        // Is a cycle running?
        bool running = false;

        nall::shared_pointer<Emulator> emulator;
        // vector<higan::Node::Screen> screens; // todo: do we need to track screens?
        vector<higan::Node::Stream> streams;

        // Frame event callbacks
        emscripten::val onFrameStart = emscripten::val::null();
        emscripten::val onFrame = emscripten::val::null();
        emscripten::val onFrameEnd = emscripten::val::null();

        auto init(uint width, uint height) -> void;
        auto load(const char *url, emscripten::val callback) -> void;
        auto run() -> void;
        auto unload() -> void;

        auto resize(uint width, uint height) -> void;

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

        auto stateSave(uint slot) -> bool;
        auto stateLoad(uint slot) -> bool;
    
    private:
        WebVideo webvideo;
        WebAudio webaudio;

        auto createJSObjectFromManifest(string& manifest) -> emscripten::val;
        auto getEmulatorByExtension(const char *extension) -> nall::shared_pointer<Emulator>;
        auto getFilenameExtension(const char *url) -> const char*;
};