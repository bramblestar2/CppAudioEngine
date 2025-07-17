#pragma once
#include <vector>
#include <mutex>

#include "Audio/Clip.h"
#include "Utility/IdPool.h"

class Mixer {
public:
    Mixer(double sampleRate, int channels);
    ~Mixer() = default;
    
    int addVoice(Clip voice);
    void removeVoice(int id);

    void process(float* out, unsigned long frames);
private:
    std::vector<std::pair<int, Clip>> m_voices;
    std::mutex m_mutex;

    double m_sampleRate;
    int m_channels;

    IdPool m_idPool;
};