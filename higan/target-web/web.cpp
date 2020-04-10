#include "web.hpp"

WebPlatform *webplatform = new WebPlatform();

/* lifecycle */
bool isStarted() { return webplatform->started; }
bool isRunning() { return webplatform->running; }

void run() { webplatform->run(); }

void onFrameStart(emscripten::val callback) {
    webplatform->onFrameStart = callback;
}

void onFrame(emscripten::val callback) {
    webplatform->onFrame = callback;
}

void onFrameEnd(emscripten::val callback) {
    webplatform->onFrameEnd = callback;
}

bool start() { 
    if (webplatform->started) {
        return false;
    }

    webplatform->started = true;
    emscripten_set_main_loop(run, 0, 0); 
    return true;
}

bool stop() { 
    if (!webplatform->started) {
        return false;
    }

    webplatform->started = false;
    emscripten_cancel_main_loop(); 
    
    while (webplatform->running) {
        emscripten_sleep(1);
    }

    return true;
}

/*  bootstrap */
void init(uint width, uint height) { return webplatform->init(width, height); }
void load(std::string url, emscripten::val callback) { return webplatform->load(url.c_str(), callback); }
void unload() { 
    if (webplatform->started) {
        stop(); 
    }
    
    return webplatform->unload(); 
}

/* configuration */
void resize(uint width, uint height) { webplatform->resize(width, height); }

// todo: audio configuration, shaders, etc.

/* controllers and peripherals */
bool connectPeripheral(std::string portName, std::string peripheralName) { return webplatform->connect(portName.c_str(), peripheralName.c_str()); }
bool disconnectPeripheral(std::string portName) { return webplatform->disconnect(portName.c_str()); }
bool setButton(std::string portName, std::string buttonName, int16_t value) {
    return webplatform->setButton(portName.c_str(), buttonName.c_str(), value);
}

/* state save, memory saves */

// Todo: return save data, load saved data instead
bool stateSave(uint slot) { return webplatform->stateSave(slot); }
bool stateLoad(uint slot) { return webplatform->stateLoad(slot); }

// Todo: snapshot save RAM(s) when data is being saved

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("init", &init);
    emscripten::function("load", &load);
    emscripten::function("unload", &unload);
    emscripten::function("run", &run);
    emscripten::function("start", &start);
    emscripten::function("stop", &stop);
    emscripten::function("isStarted", &isStarted);
    emscripten::function("isRunning", &isRunning);
    emscripten::function("onFrameStart", &onFrameStart);
    emscripten::function("onFrame", &onFrame);
    emscripten::function("onFrameEnd", &onFrameEnd);

    emscripten::function("resize", &resize);

    emscripten::function("connectPeripheral", &connectPeripheral);
    emscripten::function("disconnectconnectPeripheral", &disconnectPeripheral);
    emscripten::function("setButton", &setButton);
    
    emscripten::function("stateSave", &stateSave);
    emscripten::function("stateLoad", &stateLoad);
}