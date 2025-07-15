#pragma once
#include <cstddef>

struct Voice {
    const float* data;
    double pos = 0;
    size_t size = 0;
    // size_t frameCount = 0;

    int channels = 0;
    double sampleRate = 0.0;

    size_t frameCount() const {
        return size / channels;
    }
};