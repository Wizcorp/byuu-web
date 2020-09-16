#include "../web.hpp"
#include <icarus/icarus.hpp>

using namespace higan;

WebPlatform::WebPlatform() {
    higan::platform = this;
    Emulator::construct();
}

auto WebPlatform::initialize(const char *windowTitle) -> bool {
    DEBUG_LOG("Initializing web platform\n");
    webvideo.initialize(windowTitle);
    webaudio.initialize();
    DEBUG_LOG("Web platform initialized\n");

    return true;
};

auto WebPlatform::terminate() -> void {
    webaudio.terminate();
    webvideo.terminate();
}

auto WebPlatform::getEmulatorForFilename(const char *path) -> string {
    auto ext = getFilenameExtension(path);

    if (auto emulator = getEmulatorByExtension(ext)) {
        return emulator->name;
    }

    return "";
}

auto WebPlatform::getROMInfo(const char *path, uint8_t *rom, int size) -> emscripten::val {
    auto ext = getFilenameExtension(path);
    auto emulator = getEmulatorByExtension(ext);

    if (!emulator) {
        return emscripten::val::null();
    }

    vector<uint8_t> *data = new vector<uint8_t>();
    data->acquire(rom, size);

    for(auto& media : icarus::media) {
        if(!media->name().equals(emulator->name)) continue;
        if(auto cartridge = media.cast<icarus::Cartridge>()) {
            auto manifest = cartridge->manifest(*data, path);
            return getEmulatorAndGameInfo(emulator, manifest);
        }
  }

  return emscripten::val::null();
}

auto WebPlatform::setEmulator(const char *emulatorName) -> bool {
    this->emulator = getEmulatorByName(emulatorName);
    if(!this->emulator) {
        return false;
    }

    return true;
}

auto WebPlatform::setEmulatorForFilename(const char *path) -> bool {
    auto ext = getFilenameExtension(path);
    this->emulator = getEmulatorByExtension(ext);
    if(!this->emulator) {
        return false;
    }

    return true;
}

auto WebPlatform::load(uint8_t *rom, int size, emscripten::val files) -> emscripten::val {
    webaudio.resume();

    if (!this->emulator) {
        return emscripten::val::null(); 
    }

    // Reset games folder
    if (directory::exists(Emulator::GameFolder)) {
        if(!directory::remove(Emulator::GameFolder)) {
            DEBUG_LOG("Failed to clear game folder");
        }
    }

    if(!directory::create(Emulator::GameFolder)) {
        DEBUG_LOG("Failed to create game folder\n");
    }

    // todo: we should be able to simply keep this in memory and have the emulator access that data... going over file is pretty much useless
    if (!files.isNull() && !files.isUndefined()) {
        emscripten::val fileEntries = emscripten::val::global("Object").call<emscripten::val>("entries", files);
        int length = fileEntries["length"].as<int>();

        for (int i = 0; i < length; ++i) {
            auto filename = fileEntries[i][0].as<std::string>();
            auto filedata = fileEntries[i][1].as<std::string>();
            file::write({Emulator::GameFolder, filename.c_str()}, {filedata.c_str(), filedata.size()});
        }
    }

    vector<uint8_t> *data = new vector<uint8_t>();
    data->acquire(rom, size);

    // Todo: Remove path entirely, if possible 
    this->emulator->load("game", *data);
    return getEmulatorAndGameInfo(this->emulator, this->emulator->game.manifest);
}

auto WebPlatform::loadURL(const char *url, emscripten::val files, emscripten::val callback) -> void {
    if (!setEmulatorForFilename(url)) {
        DEBUG_LOG("No emulator found for: %s\n", url);
        callback(WebPlatform::Error::MATCHING_EMULATOR_NOT_FOUND, 0);
        return;
    }
    
    DEBUG_LOG("Using emulator: %s\n", this->emulator->name.data());

    LoadingCallbackContainer *container = new LoadingCallbackContainer(this, url, files, callback);

    DEBUG_LOG("Fetching ROM: %s\n", url);

    emscripten_async_wget_data(
        url, 
        (void *) container, 
        [](void *c, void *buffer, int size) -> void {
            LoadingCallbackContainer *container = (LoadingCallbackContainer *) c;
            auto instance = container->instance;
            auto url = container->url.data();
            auto files = container->files;
            auto callback = container->callback;

            try {
                callback(0, instance->load((uint8_t*) buffer, size, files));
            } catch (...) {
                DEBUG_LOG("Failed to load data into emulator: %s\n", url);
                callback(WebPlatform::Error::EMULATOR_ROM_LOAD_FAILED, 0);
            }
        }, 
        [](void *c) -> void {
            LoadingCallbackContainer *container = (LoadingCallbackContainer *) c;
            auto callback = container->callback;
            auto url = container->url.data();
            
            DEBUG_LOG("Failed to fetch data: %s", url);
            callback(WebPlatform::Error::ROM_FETCH_FAILED, 0);
        }
    );
}

auto WebPlatform::unload() -> void {
    if (emulator) {
        emulator->interface->unload();
        emulator = nullptr;
    }
}

auto WebPlatform::run() -> void {
    if (emulator && !running) {
        running = true;
        if (!this->onFrameStart.isNull()) {
            this->onFrameStart();
        }
        emulator->interface->run();
        if (!this->onFrameEnd.isNull()) {
            this->onFrameEnd();
        }
        running = false; 
    }
}

auto WebPlatform::configure(string name, uint value) -> void {
    if (!emulator) {
        return;
    }

    emulator->interface->configure(name, value);
}

auto WebPlatform::onResize(emscripten::val callback) -> void {
    webvideo.onResize(callback);
}

auto WebPlatform::setVolume(uint volume) -> void {
    webaudio.setVolume(volume);
};

auto WebPlatform::setMute(bool mute) -> void {
    webaudio.muted = mute;
};

auto WebPlatform::attach(Node::Object node) -> void {
    // todo: should we attach screens?
    if(auto stream = node->cast<Node::Stream>()) {
        streams = emulator->root->find<Node::Stream>();
        stream->setResamplerFrequency(webaudio.frequency);
    }
}

auto WebPlatform::detach(Node::Object node) -> void {
    // todo: should we detach screens?
    if(auto stream = node->cast<Node::Stream>()) {
        streams = emulator->root->find<Node::Stream>();
    }
}

auto WebPlatform::open(Node::Object node, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> { 
    return emulator->open(node, name, mode, required);
}

auto WebPlatform::log(string_view message) -> void {
    printf("%s\n", message.data());
}

auto WebPlatform::video(Node::Screen, const uint32_t* data, uint pitch, uint width, uint height) -> void {
    webvideo.render(data, pitch, width, height);
}

auto WebPlatform::audio(Node::Stream stream) -> void {
    if(!streams) return;

    //process all pending frames (there may be more than one waiting)
    while(true) {
        //only process a frame if all streams have at least one pending frame
        for(auto& stream : streams) {
            if(!stream->pending()) return;
        }

        //mix all frames together
        float samples[2] = {0.0, 0.0};
        for(auto& stream : streams) {
            float buffer[2];
            uint channels = stream->read(buffer);
            if(channels == 1) {
                //monaural -> stereo mixing
                samples[0] += buffer[0];
                samples[1] += buffer[0];
            } else {
                samples[0] += buffer[0];
                samples[1] += buffer[1];
            }
        }

        webaudio.output(samples);
    }
}

auto WebPlatform::connect(const string& portName, const string& peripheralName) -> bool {
    return emulator->connect(portName, peripheralName);
}

auto WebPlatform::disconnect(const string& portName) -> bool {
    return emulator->disconnect(portName);
}

auto WebPlatform::setButton(const string& portName, const string& buttonName, int16_t value) -> bool {
    return emulator->setButton(portName, buttonName, value);
}

auto WebPlatform::input(Node::Input node) -> void {
    emulator->input(node);
}

// todo: we need a callback here because it appears that
// the scheduler sync throws things for a loop, and forces
// that function to return void (false, zero, "") instead of
// the actual data being returned. 
auto WebPlatform::stateSave(emscripten::val callback) -> void {
    if(!emulator) return;

    if(auto state = emulator->interface->serialize()) {
        auto view = emscripten::typed_memory_view(state.size(), state.data());
        auto buffer = emscripten::val(view);
        callback(buffer);
    }

    return;
}

auto WebPlatform::stateLoad(const char *stateData, uint stateSize) -> bool {
    if(!emulator) return false;

    serializer state{(const uint8_t *) stateData, stateSize};
    if(emulator->interface->unserialize(state)) {
        return true;
    }

    return false;
}


auto WebPlatform::save() -> emscripten::val {
    auto files = emscripten::val::object();

    if (!emulator) {
        return files;
    }

    emulator->save();

    auto filenames = directory::files(Emulator::GameFolder);
    for (auto filename : filenames) {
        auto file = file::read({Emulator::GameFolder, filename});
        auto view = emscripten::typed_memory_view(file.size(), file.data());
        auto buffer = emscripten::val(view);
        files.set(filename.data(), buffer);
    }

    return files;
}

auto WebPlatform::getEmulatorAndGameInfo(nall::shared_pointer<Emulator> emulator, string manifest) -> emscripten::val {
    uint index;
    auto ports = emscripten::val::array();
    
    index = 0;
    for (auto port : emulator->ports) {
        ports.set(index, port.data());
        index++;
    }

    auto buttons = emscripten::val::array();
    index = 0;
    for (auto button : emulator->buttons) {
        buttons.set(index, button.data());
        index++;
    }

    auto emulatorInfo = emscripten::val::object();
    emulatorInfo.set("name", emulator->name.data());
    emulatorInfo.set("ports", ports);
    emulatorInfo.set("buttons", buttons);

    auto info = emscripten::val::object();
    info.set("emulator", emulatorInfo);
    info.set("rom", createJSObjectFromManifest(manifest));

    return info;
}

auto WebPlatform::createJSObjectFromManifest(string& manifest) -> emscripten::val {
    auto node = BML::unserialize(manifest);
    return parseBMLNode(node);
}

// See: /nall/string/markup/bml.hpp
// See: https://byuu.org/docs/higan/manifests
// See: https://byuu.org/preservation/boards-(production)
//
// Note: values are converted to std::string because const char *
// does not return UTF-8 strings to JavaScript
auto WebPlatform::parseBMLNode(Markup::Node node) -> emscripten::val {
    if(!node.name()) {
        auto arr = emscripten::val::array();
        auto index = 0;
        for(auto leaf : node) {
            arr.set(index, parseBMLNode(leaf));
            index++;
        }

        return arr;
    }

    auto name = node.name().data();
    auto data = emscripten::val::object();
    data.set("name", name);
    
    vector<string> lines;
    if(auto value = node.value()) {
        lines = value.split("\n");
    }

    if(lines.size() == 0) {
        data.set("value", emscripten::val::null());
    } else if(lines.size() == 1) {
        std::string value = lines[0].trimLeft(" ").data();
        data.set("value", value);
    } else {
        auto attr = emscripten::val::array();
        auto index = 0;
        for (auto line : lines) {
            std::string value =  line.trimLeft(" ").data();
            attr.set(index, value);
            index++;
        }
        data.set("value", attr);
    }

    auto val = emscripten::val::array();
    auto index = 0;
    for(auto leaf : node) {
        val.set(index, parseBMLNode(leaf));
        index++;
    }
    data.set("children", val);

    return data;
}

auto WebPlatform::getEmulatorByName(const char *name) -> nall::shared_pointer<Emulator> {
    for(auto& emulator : emulators) {
        if(emulator->name.equals(name)) {
            return emulator;
        }
    }

    return nullptr;
}

auto WebPlatform::getEmulatorByExtension(const char *ext) -> nall::shared_pointer<Emulator> {
    for(auto& emulator : emulators) {
        for(auto& extension : emulator->extensions) {
            if(extension.equals(ext)) {
                return emulator;
            }
        }
    }

    return nullptr;
}

auto WebPlatform::getFilenameExtension(const char *url) -> const char* {
    const char *dot = strrchr(url, '.');

    if(!dot || dot == url) {
        return "";
    };

    return dot + 1;
}