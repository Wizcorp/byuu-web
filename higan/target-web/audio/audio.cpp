#include "../web.hpp"

void WebAudio::initialize() {
    // Audio
    if (!device) {
        device = alcOpenDevice(NULL);

        //Create a context
        context = alcCreateContext(device, NULL);
        
        //Set active context
        alcMakeContextCurrent(context);

        // generate audio source
        alGenSources(1, &source);

        alListener3f(AL_POSITION, 0.0, 0.0, 0.0);
        alListener3f(AL_VELOCITY, 0.0, 0.0, 0.0);
        ALfloat listenerOrientation[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        alListenerfv(AL_ORIENTATION, listenerOrientation);
    }

    uint size = frequency * latency / 1000.0 + 0.5;

    if (size != bufferSize) {
        free(buffer);
        bufferSize = size;
        buffer = new uint32_t[bufferSize]();
    }
}

void WebAudio::output(double samples[2]) {
    buffer[bufferLength]  = (uint16_t)sclamp<16>(samples[0] * 32767.0) <<  0;
    buffer[bufferLength] |= (uint16_t)sclamp<16>(samples[1] * 32767.0) << 16;
    
    if(++bufferLength < bufferSize) return;

    ALuint alBuffer = 0;
    int processed = 0;
    while(true) {
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
        while(processed--) {
            alSourceUnqueueBuffers(source, 1, &alBuffer);
            alDeleteBuffers(1, &alBuffer);
            queueLength--;
        }
        //wait for buffer playback to catch up to sample generation if not synchronizing
        if(!blocking || queueLength < 3) break;
    }

    if(queueLength < 3) {
        alGenBuffers(1, &alBuffer);
        alBufferData(alBuffer, AL_FORMAT_STEREO16, buffer, bufferSize * 4, frequency);
        alSourceQueueBuffers(source, 1, &alBuffer);
        queueLength++;
    }

    ALint playing;
    alGetSourcei(source, AL_SOURCE_STATE, &playing);
    if(playing != AL_PLAYING) alSourcePlay(source);
    bufferLength = 0;
}