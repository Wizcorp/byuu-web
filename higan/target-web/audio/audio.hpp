#include <AL/al.h>
#include <AL/alc.h>

struct WebAudio {
	bool initialized = false;

	ALCdevice *device = nullptr;
	ALCcontext *context = nullptr;
	ALuint source = 0;

	uint frequency;
	uint latency = 120;
	uint volume = 100;

	bool muted = false;
	bool blocking = false;

	uint queueLength = 0;
	uint32_t* buffer = nullptr;
	uint bufferLength = 0;
	uint bufferSize = 0;

	void initialize();
	void terminate();

	bool resume();
	void setVolume(uint volume);
	void output(float samples[2]);
};