#pragma once
#include <samplerate.h>
#include <vector>
#include <string>
#include <cstdint>

std::vector<float> resampleBuffer(std::vector<float>& data, double inputSampleRate, double outputSampleRate, int channels, int converterType = SRC_SINC_MEDIUM_QUALITY);

struct AudioBuffer {
    std::vector<float> data;
    int channels;
    double sampleRate;

    AudioBuffer(std::vector<float>&& data, int channels, double sampleRate);

    uint64_t frameCount() const {
        return data.size() / std::max(1, channels);
    }

    float estimatedBPM();
    void calculateBPM();

    static AudioBuffer load(std::string file);
    static AudioBuffer load(std::string file, uint64_t start, uint64_t end = 0, double sampleRate = -1);
private:
    float beatsPerMinute;
};