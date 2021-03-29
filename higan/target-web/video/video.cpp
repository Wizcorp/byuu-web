#include "../web.hpp"

void WebVideo::initialize(const char* windowTitle) {
    if (!renderer) {
        SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "0");

        DEBUG_LOG("Initializing video\n");

        // Disable event handler setup
        EM_ASM({
             window.realHandler = JSEvents.registerOrRemoveHandler;
             JSEvents.registerOrRemoveHandler = () => true;
        });

        assert(SDL_Init(SDL_INIT_VIDEO) == 0);
        window = SDL_CreateWindow(windowTitle, 0, 0, width, height, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        // Re-enable event handler setup
        EM_ASM({
             JSEvents.registerOrRemoveHandler = window.realHandler;
             delete window.realHandler;
        });
    }
}

void WebVideo::terminate() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void WebVideo::render(const void *data, uint pitch, uint frameWidth, uint frameHeight, double screenScaleX, double screenScaleY) {
    if (
        texture 
        && (
            width != frameWidth 
            || height != frameHeight 
            || fabs(screenScaleX - scaleX) > 0.1
            || fabs(screenScaleY - scaleY) > 0.1
        )
    ) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    if (texture == nullptr) {
        width = frameWidth; 
        height = frameHeight;
        scaleX = screenScaleX;
        scaleY = screenScaleY;
        
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, frameWidth, frameHeight);

        // make screen size double the texture size to increase the render quality;
        // additional stretching should be done in CSS by the end-user
        scaledWidth = frameWidth * 2; 
        scaledHeight = frameHeight * 2;

        if (scaleX != 0 && scaleY != 0) {
            double ratio = scaleX / scaleY;
        
            if (ratio > 1) {
                scaledWidth *= ratio;
            } else {
                scaledHeight *= (1/ratio);
            }
        }

        SDL_SetWindowSize(window, scaledWidth, scaledHeight);

        if (!onResizeCallback.isNull()) {
            onResizeCallback(emscripten::val(scaledWidth), emscripten::val(scaledHeight));
        }
    }
    
    SDL_UpdateTexture(texture, nullptr, data, pitch);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

auto WebVideo::onResize(emscripten::val callback) -> void {
    onResizeCallback = callback;
}