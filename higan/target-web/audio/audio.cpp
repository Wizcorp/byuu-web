#include "../web.hpp"

void WebAudio::initialize() {
    // Audio
    if (!device) {
        device = alcOpenDevice(NULL);
    }

    if (!context) {
        //Create a context
        context = alcCreateContext(device, NULL);
        
        //Set active context
        alcMakeContextCurrent(context);

        // generate audio source
        alGenSources(1, &source);

        setVolume(volume);
        alListener3f(AL_POSITION, 0.0, 0.0, 0.0);
        alListener3f(AL_VELOCITY, 0.0, 0.0, 0.0);
        ALfloat listenerOrientation[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        alListenerfv(AL_ORIENTATION, listenerOrientation);

        // This bit of code is needed to allow iOS to play the audio
        // Note: previous implementations included a hack to allow audio even on silent mode
        // See: https://github.com/Wizcorp/byuu-web/commit/1c28542c7a2c68fcd57332516d361cec18f3dfef
        // See: https://stackoverflow.com/a/46839941/262831
        EM_ASM({
            window.addEventListener('touchstart', () => {
                const { audioCtx } = AL.currentCtx;
                if(audioCtx.state == 'suspended') {
                    audioCtx.resume();
                }
            }, { once: true });
        });
    }

    // See: https://emscripten.org/docs/porting/Audio.html#useful-implementation-details-of-openal-capture
    if (!frequency) {
        frequency = EM_ASM_INT({
            var AudioContext = window.AudioContext || window.webkitAudioContext;
            var ctx = new AudioContext();
            var sr = ctx.sampleRate;
            ctx.close();
            return sr;
        });
    }

    uint size = frequency * latency / 1000.0 + 0.5;

    if (size != bufferSize) {
        delete [] buffer;
        bufferSize = size;
        buffer = new uint32_t[bufferSize]();
    }
}

void WebAudio::setVolume(uint volume) {
    alSourcef(source, AL_GAIN, (float) volume / 100.0f);
    this->volume = volume;
}

void WebAudio::terminate() {    
    alSourceStop(source);
    bufferLength = 0;
}

bool WebAudio::resume() {
    return EM_ASM_INT({
        if(!AL || !AL.currentCtx) {
            return 0;
        }

        const { audioCtx } = AL.currentCtx;
        if(audioCtx.state == 'suspended') {
            audioCtx.resume();
        }

        return 1;
    });
}

void WebAudio::output(float samples[2]) {
    if (muted || device == nullptr || context == nullptr) {
        return;
    }

    buffer[bufferLength]  = (uint16_t)sclamp<16>(samples[0] * 32767.0) <<  0;
    buffer[bufferLength] |= (uint16_t)sclamp<16>(samples[1] * 32767.0) << 16;
    
    if(++bufferLength < bufferSize) {
        return;
    }

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