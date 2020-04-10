#define DEBUG_LOG(...) printf(__VA_ARGS__)

#include <emulator/emulator.hpp>

#include <nall/instance.hpp>
#include <nall/hid.hpp>
#include <nall/beat/single/apply.hpp>
#include <nall/map.hpp>

#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/fetch.h>

#include "resource/resource.hpp"
#include "emulator/emulator.hpp"
#include "video/video.hpp"
#include "audio/audio.hpp"
#include "platform/platform.hpp"
