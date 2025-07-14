#pragma once
#include <vector>
#include <string>

#include "Audio/AudioBuffer.h"


class AudioManager {
public:
    AudioManager();

    void load(std::string file, int id = -1);
    void load(std::vector<float>& data, int channels, double sampleRate, int id = -1);
    void load(AudioBuffer& audioBuffer, int id = -1);
    void remove(int id);

    std::pair<int, AudioBuffer>* get(int index) const;
    int size() const noexcept;

private:
    std::vector<std::pair<int, AudioBuffer>> m_audioBuffers;
};