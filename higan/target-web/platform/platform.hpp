struct WebPlatform;

struct LoadingCallbackContainer {
    LoadingCallbackContainer(WebPlatform *i, const char *u, emscripten::val c) : instance(i), url(u), callback(c) {}; 
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
        nall::shared_pointer<Emulator> emulator;
        // vector<higan::Node::Screen> screens; // todo: do we need to track screens?
        vector<higan::Node::Stream> streams;

        WebVideo webvideo;
        WebAudio webaudio;

        auto init() -> void;
        auto load(const char *url, emscripten::val callback) -> void;
        auto run() -> void;
        auto unload() -> void;

        auto attach(Object object) -> void override;
        auto detach(Object object) -> void override;
        auto open(
            Object node, 
            string name, 
            vfs::file::mode mode, 
            bool required = false
        ) -> shared_pointer<vfs::file> override;
        auto event(Event) -> void override;
        auto log(string_view message) -> void override;
        auto video(Screen, const uint32_t* data, uint pitch, uint width, uint height) -> void override;
        auto audio(Stream) -> void override;
        auto input(Input) -> void override;
    
    private:
        auto getEmulatorByExtension(const char *extension) -> nall::shared_pointer<Emulator>;
        auto getFilenameExtension(const char *url) -> const char*;
};