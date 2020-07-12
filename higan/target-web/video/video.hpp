#include <SDL2/SDL.h>

struct WebVideo {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture = nullptr;

	uint width = 256;
	uint height = 224;

    void initialize(const char *windowTitle, uint width, uint height);
	void terminate();
	void resize(uint width, uint height);
    void render(const void *data, uint pitch, uint frameWidth, uint frameHeight);
};