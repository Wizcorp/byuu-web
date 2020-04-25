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

        // This bit of code is needed to allow iOS to play the audio even when silence mode is on (mute button)
        // See: https://stackoverflow.com/a/46839941/262831
        EM_ASM({
            const unlock = () => {
                const isUnlocked = () => this.isWebAudioUnlocked  && this.isHTMLAudioUnlocked;
                const removeEventListener = () => {
                    if (isUnlocked()) {
                        document.body.removeEventListener('touchstart', unlock);
                    }
                };

                if (isUnlocked()) return;

                // Unlock WebAudio - create short silent buffer and play it
                // This will allow us to play web audio at any time in the app
                const { audioCtx } = AL.currentCtx;
                const buffer = audioCtx.createBuffer(1, 1, 22050); // 1/10th of a second of silence
                const source = audioCtx.createBufferSource();
                Object.assign(source, { buffer, onended: () => removeEventListener(this.isWebAudioUnlocked = true) });
                source.connect(audioCtx.destination);
                source.start();

                // Unlock HTML5 Audio - load a data url of short silence and play it
                // This will allow us to play web audio when the mute toggle is on
                const silenceDataURL = 'data:audio/mp3;base64,//MkxAAHiAICWABElBeKPL/RANb2w+yiT1g/gTok//lP/W/l3h8QO/OCdCqCW2Cw//MkxAQHkAIWUAhEmAQXWUOFW2dxPu//9mr60ElY5sseQ+xxesmHKtZr7bsqqX2L//MkxAgFwAYiQAhEAC2hq22d3///9FTV6tA36JdgBJoOGgc+7qvqej5Zu7/7uI9l//MkxBQHAAYi8AhEAO193vt9KGOq+6qcT7hhfN5FTInmwk8RkqKImTM55pRQHQSq//MkxBsGkgoIAABHhTACIJLf99nVI///yuW1uBqWfEu7CgNPWGpUadBmZ////4sL//MkxCMHMAH9iABEmAsKioqKigsLCwtVTEFNRTMuOTkuNVVVVVVVVVVVVVVVVVVV//MkxCkECAUYCAAAAFVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV';
                const tag = document.createElement('audio');
                Object.assign(tag, { controls: false, preload: 'auto', loop: false, src: silenceDataURL, onended: () => removeEventListener(this.isWebAudioUnlocked = true) });
                const p = tag.play();
                // if (p) p.then(function(){console.log("play success")}, function(reason){console.log("play failed", reason)});
            };

            document.body.addEventListener('touchstart', unlock);
        });
    }

    uint size = frequency * latency / 1000.0 + 0.5;

    if (size != bufferSize) {
        free(buffer);
        bufferSize = size;
        buffer = new uint32_t[bufferSize]();
    }
}

bool WebAudio::resume() {
    return EM_ASM_INT({
        if(!AL) {
            return 0;
        }

        const { audioCtx } = AL.currentCtx;
        if(audioCtx.state == 'suspended') {
            audioCtx.resume();
        }

        return 1;
    });
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