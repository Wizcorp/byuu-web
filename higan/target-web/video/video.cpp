#include "../web.hpp"

void WebVideo::initialize(const char* windowTitle, uint width, uint height) {
    if (!renderer) {
        DEBUG_LOG("Initializing video\n");

        assert(SDL_Init(SDL_INIT_VIDEO) == 0);
        window = SDL_CreateWindow(windowTitle, 0, 0, width, height, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    }
}

void WebVideo::resize(uint width, uint height) {
    SDL_SetWindowSize(window, width, height);
}

void WebVideo::render(const void *data, uint pitch, uint frameWidth, uint frameHeight) {
    if (texture && (width != frameWidth || height != frameHeight)) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    if (texture == nullptr) {
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, frameWidth, frameHeight);
        width = frameWidth; 
        height = frameHeight;
    }
    
    SDL_UpdateTexture(texture, nullptr, data, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}