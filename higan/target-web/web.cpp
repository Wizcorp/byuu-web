#include "web.hpp"

WebPlatform *webplatform = new WebPlatform();
emscripten::val scheduledStateSave = emscripten::val::null();

/* lifecycle */
bool isStarted() { return webplatform->started; }
bool isRunning() { return webplatform->running; }

void run() {
    webplatform->run();

    if (!scheduledStateSave.isNull()) {
        webplatform->stateSave(scheduledStateSave); 
        scheduledStateSave = emscripten::val::null();
    }

    // Cancel main loop if we are not running
    if (!webplatform->started) {
        emscripten_cancel_main_loop(); 
    }
}

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
    
    return true;
}

void unload() { 
    if (webplatform->started) {
        stop(); 
    }
    
    return webplatform->unload(); 
}

/*  bootstrap */
bool initialize(std::string windowTitle) { return webplatform->initialize(windowTitle.c_str()); }
void terminate() { return webplatform->terminate(); }

std::string getEmulatorForFilename(std::string path) { return webplatform->getEmulatorForFilename(path.c_str()).data(); };
emscripten::val getROMInfo(std::string path, std::string rom) { return webplatform->getROMInfo(path.c_str(), (uint8_t *) rom.c_str(), rom.size()); }

bool setEmulator(std::string emulatorName) { return webplatform->setEmulator(emulatorName.c_str()); }
bool setEmulatorForFilename(std::string path) { return webplatform->setEmulatorForFilename(path.c_str()); }

emscripten::val load(std::string rom, emscripten::val files) { return webplatform->load((uint8_t *) rom.c_str(), rom.size(), files); }
void loadURL(std::string url, emscripten::val files, emscripten::val callback) { return webplatform->loadURL(url.c_str(), files, callback); }

/* configuration */
void configure(std::string name, uint8_t value) { return webplatform->configure(name.c_str(), value); }
void onResize(emscripten::val callback) { return webplatform->onResize(callback); }
void setVolume(uint volume) { webplatform->setVolume(volume); }
void setMute(bool mute) { webplatform->setMute(mute); }

/* controllers and peripherals */
bool connectPeripheral(std::string portName, std::string peripheralName) { return webplatform->connect(portName.c_str(), peripheralName.c_str()); }
bool disconnectPeripheral(std::string portName) { return webplatform->disconnect(portName.c_str()); }
bool setButton(std::string portName, std::string buttonName, int16_t value) {
    return webplatform->setButton(portName.c_str(), buttonName.c_str(), value);
}

/* state save, memory saves */
void stateSave(emscripten::val callback) { 
    scheduledStateSave = callback;

    // Run one cycle then stop
    if (!webplatform->started) {
        emscripten_set_main_loop(run, 0, 0);
    }
}

bool stateLoad(std::string state) { return webplatform->stateLoad(state.c_str(), state.size()); }
emscripten::val save() { return webplatform->save(); }

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("initialize", &initialize);
    emscripten::function("terminate", &terminate);
    emscripten::function("getEmulatorForFilename", &getEmulatorForFilename);
    emscripten::function("setEmulator", &setEmulator);
    emscripten::function("setEmulatorForFilename", &setEmulatorForFilename);

    emscripten::function("load", &load);
    emscripten::function("loadURL", &loadURL);
    emscripten::function("unload", &unload);
    emscripten::function("configure", &configure);
    emscripten::function("run", &run);
    emscripten::function("start", &start);
    emscripten::function("stop", &stop);
    
    emscripten::function("isStarted", &isStarted);
    emscripten::function("isRunning", &isRunning);
    
    emscripten::function("onFrameStart", &onFrameStart);
    emscripten::function("onFrameEnd", &onFrameEnd);
    emscripten::function("onResize", &onResize);
    
    emscripten::function("setVolume", &setVolume);
    emscripten::function("setMute", &setMute);

    emscripten::function("connectPeripheral", &connectPeripheral);
    emscripten::function("disconnectconnectPeripheral", &disconnectPeripheral);
    emscripten::function("setButton", &setButton);
    
    emscripten::function("getROMInfo", &getROMInfo);
    
    emscripten::function("stateSave", &stateSave);
    emscripten::function("stateLoad", &stateLoad);
    emscripten::function("save", &save);
}