#pragma once
#include <vector>

struct AudioData {
    std::vector<float> data;
    int channels;
    double sampleRate;
};