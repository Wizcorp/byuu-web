#include "../web.hpp"

using higan::Node::Object;
using higan::Node::Screen;
using higan::Node::Stream;
using higan::Node::Input;
using higan::Event;

auto WebPlatform::init() -> void {
    DEBUG_LOG("Initializing web platform\n");
    higan::platform = this;
    Emulator::construct();
    webvideo.init();
};

auto WebPlatform::load(const char *url, emscripten::val callback) -> void {
    // todo: We should probably do something better than wait for user input 
    // to trigger audio setup
    webaudio.init();

    auto extension = getFilenameExtension(url);
    this->emulator = getEmulatorByExtension(extension);

    if (!this->emulator) {
        DEBUG_LOG("No emulator found for: %s\n", url);
        callback(WebPlatform::Error::MATCHING_EMULATOR_NOT_FOUND);
        return;
    }
    
    DEBUG_LOG("Using emulator: %s\n", this->emulator->name.data());

    LoadingCallbackContainer *container = new LoadingCallbackContainer(this, url, callback);

    DEBUG_LOG("Fetching ROM: %s\n", url);

    emscripten_async_wget_data(
        url, 
        (void *) container, 
        [](void *c, void *buffer, int size) -> void {
            LoadingCallbackContainer *container = (LoadingCallbackContainer *) c;
            vector<uint8_t> *data = new vector<uint8_t>();
            data->acquire((uint8_t*)buffer, size);

            try {
                container->instance->emulator->load(container->url, *data);
                DEBUG_LOG("Data loaded and emulator ready: %s\n", container->url);
                container->callback();
            } catch (...) {
                DEBUG_LOG("Failed to load data into emulator: %s\n", container->url);
                container->callback(WebPlatform::Error::EMULATOR_ROM_LOAD_FAILED);
            }
        }, 
        [](void *c) -> void {
            LoadingCallbackContainer *container = (LoadingCallbackContainer *) c;
            DEBUG_LOG("Failed to fetch data: %s", container->url);
            container->callback(WebPlatform::Error::ROM_FETCH_FAILED);
        }
    );
}

auto WebPlatform::run() -> void {
    // todo: handle case when emulator is not set
    emulator->interface->run();
}

auto WebPlatform::unload() -> void {
    if (emulator) {
        emulator->interface->unload();
        emulator = nullptr;
    }
}

auto WebPlatform::attach(Object node) -> void {
    // todo: should we attach screens?
    if(auto stream = node->cast<Stream>()) {
        streams = emulator->root->find<Stream>();
        stream->setResamplerFrequency(webaudio.frequency);
    }
}

auto WebPlatform::detach(Object node) -> void {
    // todo: should we detach screens?
    if(auto stream = node->cast<Stream>()) {
        streams = emulator->root->find<Stream>();
    }
}

auto WebPlatform::open(Object node, string name, vfs::file::mode mode, bool required) -> shared_pointer<vfs::file> { 
    return emulator->open(node, name, mode, required);
}

auto WebPlatform::event(Event) -> void {
    // todo: do we even need this?
}

auto WebPlatform::log(string_view message) -> void {
    printf("%s\n", message.data());
}

auto WebPlatform::video(Screen, const uint32_t* data, uint pitch, uint width, uint height) -> void {
    webvideo.render(data, pitch, width, height);
}

auto WebPlatform::audio(Stream stream) -> void {
    if(!streams) return;

    //process all pending frames (there may be more than one waiting)
    while(true) {
        //only process a frame if all streams have at least one pending frame
        for(auto& stream : streams) {
            if(!stream->pending()) return;
        }

        //mix all frames together
        double samples[2] = {0.0, 0.0};
        for(auto& stream : streams) {
            double buffer[2];
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

        //apply volume, balance, and clamping to the output frame
        double volume = webaudio.volume / 100;
        double balance = 0; // todo

        for(uint c : range(2)) {
            samples[c] = max(-1.0, min(+1.0, samples[c] * volume));
            if(balance < 0.0) samples[1] *= 1.0 + balance;
            if(balance > 0.0) samples[0] *= 1.0 - balance;
        }

        webaudio.output(samples);
    }
}

auto WebPlatform::input(Input input) -> void {
    // todo
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