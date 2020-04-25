#include <AL/al.h>
#include <AL/alc.h>

struct WebAudio {
	bool initialized = false;

	ALCdevice *device;
	ALCcontext *context;
	ALuint source = 0;

	uint frequency = 48000;
	uint latency = 100;
	uint volume = 100;

	bool muted = false;
	bool blocking = false;

	uint queueLength = 0;
	uint32_t* buffer = nullptr;
	uint bufferLength = 0;
	uint bufferSize = 0;

	void initialize();
	bool resume();
	void output(double samples[2]);
};