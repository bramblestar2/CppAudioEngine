#pragma once
#include <mutex>
#include <vector>

#include <portaudio.h>

#include "Audio/AudioManager.h"
#include "Audio/Mixer.h"

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    int addClip(Clip clip);
    void removeClip(int id);

    bool start();
    bool stop();

    static bool init();
    static bool shutdown();

private:
    static int paCallback(const void* in, void* out, 
                          unsigned long frames, 
                          const PaStreamCallbackTimeInfo* timeInfo, 
                          PaStreamCallbackFlags flags, void* userData);

    int handleStreamCallback(const void* in, void* out, 
                              unsigned long frames, 
                              const PaStreamCallbackTimeInfo* timeInfo, 
                              PaStreamCallbackFlags flags);
          
    float m_sampleRate;
    int m_channels;

    Mixer m_mixer;
    PaStream *m_stream;
};