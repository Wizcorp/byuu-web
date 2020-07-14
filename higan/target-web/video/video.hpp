#include <SDL2/SDL.h>

struct WebVideo {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture = nullptr;

	uint width = 256;
	uint height = 224;

	emscripten::val onResize = emscripten::val::null();

    void initialize(const char *windowTitle);
	void terminate();
    void render(const void *data, uint pitch, uint frameWidth, uint frameHeight);

	auto whenResize(emscripten::val callback) -> void;
};