#pragma once
#include <vector>

struct AudioBuffer {
    std::vector<float> data;
    int channels;
    double sampleRate;

    AudioBuffer(std::vector<float>&& data, int channels, double sampleRate);

    float estimatedBPM();
    void calculateBPM();
private:
    float beatsPerMinute;
};