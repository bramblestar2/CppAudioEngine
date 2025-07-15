#pragma once
#include <vector>

struct AudioBuffer {
    std::vector<float> data;
    int channels;
    double sampleRate;

    AudioBuffer(std::vector<float>&& data, int channels, double sampleRate);

    float estimatedBPM();
    void calculateBPM();

    static AudioBuffer load(std::string file);
    static AudioBuffer load(std::string file, double start = 0, double end = 0, double sampleRate = -1, int channels = -1);
private:
    float beatsPerMinute;
};