#include "web.hpp"

WebPlatform *webplatform = new WebPlatform();

/* lifecycle */
bool isStarted() { return webplatform->started; }
bool isRunning() { return webplatform->running; }

void run() { webplatform->run(); }

void onFrameStart(emscripten::val callback) {
    webplatform->onFrameStart = callback;
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

void unload() { 
    if (webplatform->started) {
        stop(); 
    }
    
    return webplatform->unload(); 
}

/*  bootstrap */
bool initialize(uint width, uint height) { return webplatform->initialize(width, height); }

std::string getEmulatorForFilename(std::string path) { return webplatform->getEmulatorForFilename(path.c_str()).data(); };
emscripten::val getROMInfo(std::string path, std::string rom) { return webplatform->getROMInfo(path.c_str(), (uint8_t *) rom.c_str(), rom.size()); }

bool setEmulator(std::string emulatorName) { return webplatform->setEmulator(emulatorName.c_str()); }
bool setEmulatorForFilename(std::string path) { return webplatform->setEmulatorForFilename(path.c_str()); }

emscripten::val load(std::string rom, emscripten::val files) { 
    return webplatform->load((uint8_t *) rom.c_str(), rom.size(), files); 
}
void loadURL(std::string url, emscripten::val files, emscripten::val callback) { return webplatform->loadURL(url.c_str(), files, callback); }


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
void stateSave(emscripten::val callback) { return webplatform->stateSave(callback); }
bool stateLoad(std::string state) { return webplatform->stateLoad(state.c_str(), state.size()); }
emscripten::val save() { return webplatform->save(); }

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("initialize", &initialize);
    emscripten::function("getEmulatorForFilename", &getEmulatorForFilename);
    emscripten::function("setEmulator", &setEmulator);
    emscripten::function("setEmulatorForFilename", &setEmulatorForFilename);

    emscripten::function("load", &load);
    emscripten::function("loadURL", &loadURL);
    emscripten::function("unload", &unload);
    emscripten::function("run", &run);
    emscripten::function("start", &start);
    emscripten::function("stop", &stop);
    
    emscripten::function("isStarted", &isStarted);
    emscripten::function("isRunning", &isRunning);
    
    emscripten::function("onFrameStart", &onFrameStart);
    emscripten::function("onFrameEnd", &onFrameEnd);

    emscripten::function("resize", &resize);

    emscripten::function("connectPeripheral", &connectPeripheral);
    emscripten::function("disconnectconnectPeripheral", &disconnectPeripheral);
    emscripten::function("setButton", &setButton);
    
    emscripten::function("getROMInfo", &getROMInfo);
    
    emscripten::function("stateSave", &stateSave);
    emscripten::function("stateLoad", &stateLoad);
    emscripten::function("save", &save);
}