#include "web.hpp"

WebPlatform *webplatform = new WebPlatform();

void run() { webplatform->run(); }
void start() {  emscripten_set_main_loop(run, 0, 0); }
void stop() { emscripten_cancel_main_loop(); }

void init() { webplatform->init(); }
void load(std::string url, emscripten::val callback) { webplatform->load(url.c_str(), callback); }
void unload() { stop(); webplatform->unload(); }

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("init", &init);
    emscripten::function("load", &load);
    emscripten::function("unload", &unload);
    emscripten::function("run", &run);
    emscripten::function("start", &start);
    emscripten::function("stop", &stop);
}