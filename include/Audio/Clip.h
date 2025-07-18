#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "Audio/Effects/ClipEffects.h"

struct Clip {
    const float* data;
    double pos = 0;
    uint64_t size = 0;

    int channels = 0;
    double sampleRate = 0.0;

    std::vector<std::shared_ptr<ClipEffect>> effects;

    uint64_t frameCount() const {
        return size / channels;
    }

    static Clip fromBuffer(class AudioBuffer& buffer, uint64_t start = 0 /* ms */, uint64_t end = 0 /* ms */);
};