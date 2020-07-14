#include "../web.hpp"

void WebVideo::initialize(const char* windowTitle) {
    if (!renderer) {
        DEBUG_LOG("Initializing video\n");

        assert(SDL_Init(SDL_INIT_VIDEO) == 0);
        window = SDL_CreateWindow(windowTitle, 0, 0, width, height, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    }
}

void WebVideo::terminate() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void WebVideo::render(const void *data, uint pitch, uint frameWidth, uint frameHeight) {
    if (texture && (width != frameWidth || height != frameHeight)) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    if (texture == nullptr) {
        SDL_SetWindowSize(window, frameWidth, frameHeight);
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, frameWidth, frameHeight);
        width = frameWidth; 
        height = frameHeight;

        if (!onResize.isNull()) {
            onResize(emscripten::val(width), emscripten::val(height));
        }
    }
    
    SDL_UpdateTexture(texture, nullptr, data, pitch);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

auto WebVideo::whenResize(emscripten::val callback) -> void {
    onResize = callback;
}