#pragma once
#include <mutex>
#include <vector>

#include <portaudio.h>

#include "Audio/AudioManager.h"
#include "Audio/Voice.h"

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    void addVoice(Voice&& voice);
    void removeVoice(int id);

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

    PaStream *m_stream;

    std::mutex m_mutex;

    std::vector<Voice> m_voices;

    float m_sampleRate;
    int m_channels;
};