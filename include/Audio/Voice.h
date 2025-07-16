#pragma once
#include <cstdint>

struct Voice {
    const float* data;
    double pos = 0;
    uint64_t size = 0;
    // size_t frameCount = 0;

    int channels = 0;
    double sampleRate = 0.0;

    uint64_t frameCount() const {
        return size / channels;
    }

    static Voice fromBuffer(class AudioBuffer& buffer, uint64_t start = 0 /* ms */, uint64_t end = 0 /* ms */);
};