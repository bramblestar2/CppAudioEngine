#pragma once
#include <vector>
#include <string>

#include "Audio/AudioBuffer.h"


class AudioManager {
public:
    AudioManager();

    int load(std::string file, int id = -1);
    int load(std::vector<float>&& data, int channels, double sampleRate, int id = -1);
    int load(AudioBuffer&& audioBuffer, int id = -1);
    
    void remove_id(int id);
    void remove(int index);
    void clear();

    AudioBuffer* get_id(int id);
    std::pair<int, AudioBuffer>* get(int index);
    int size() const noexcept;

    bool save(std::string db);
    bool open(std::string db);


private:
    std::vector<std::pair<int, AudioBuffer>> m_audioBuffers;
};