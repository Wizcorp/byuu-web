#include <SDL2/SDL.h>

struct WebVideo {
	SDL_Window *window = nullptr;
	SDL_Renderer *renderer = nullptr;
	SDL_Texture *texture = nullptr;

	uint width = 256;
	uint height = 224;
	double scaleX = 1;
	double scaleY = 1;

	double scaledWidth = width * 2;
	double scaledHeight = height * 2;

	emscripten::val onResizeCallback = emscripten::val::null();

    void initialize(const char *windowTitle);
	void terminate();
    void render(const void *data, uint pitch, uint frameWidth, uint frameHeight, double scaleX, double scaleY);

	auto onResize(emscripten::val callback) -> void;
};