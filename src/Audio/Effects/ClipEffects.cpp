#include "Audio/Effects/ClipEffects.h"
#include <cstdint>
#include "Audio/Clip.h"

#include <spdlog/spdlog.h>

void ClipEffect::apply(Clip& clip, float* out, unsigned long frames) {}

double ClipEffect::modifyStep(double baseStep) {
    return baseStep;
}


ClipFadeIn::ClipFadeIn(double durationSeconds,
                       std::pair<double,double> cp1,
                       std::pair<double,double> cp2) 
    : m_duration(durationSeconds)
    , m_cp1(cp1)
    , m_cp2(cp2)
{}

void ClipFadeIn::apply(Clip& clip, float* out, unsigned long frames) {
    const auto fadeFrames = static_cast<uint64_t>(m_duration * clip.sampleRate);
    
    if (fadeFrames == 0) {
        return;
    }
    
    const int ch = clip.channels;
    uint64_t start = uint64_t(clip.pos);

    for (uint64_t f = 0; f < frames && f < fadeFrames; ++f) {
        uint64_t idx = start + f;
        double t = double(idx) / double(fadeFrames);

        if (t > 1.0) t = 1.0;

        double u = 1.0 - t;

        double gain = 
            u * u * u * 0.0
        +   3 * u * u * t * m_cp1.second
        +   3 * u * t * t * m_cp2.second
        +   t * t * t * 1.0;

        for (int c = 0; c < ch; ++c) {
            out[f * ch + c] *= static_cast<float>(gain);
        }
    }
}

std::string ClipFadeIn::name() const {
    return "FadeIn";
}


ClipFadeOut::ClipFadeOut(double durationSeconds,
                         std::pair<double,double> cp1,
                         std::pair<double,double> cp2) 
    : m_duration(durationSeconds)
    , m_cp1(cp1)
    , m_cp2(cp2)
{}

void ClipFadeOut::apply(class Clip& clip, float* out, unsigned long frames) {
    const uint64_t totalFrames = clip.frameCount();
    const auto fadeFrames = static_cast<uint64_t>(m_duration * clip.sampleRate);
    
    if (fadeFrames == 0 || fadeFrames >= totalFrames) {
        return;
    }

    const uint64_t fadeStartFrame = totalFrames - fadeFrames;
    
    const int ch = clip.channels;
    const uint64_t start = uint64_t(clip.pos);

    for (uint64_t f = 0; f < frames && f < fadeFrames; ++f) {
        uint64_t idx = start + f;

        if (idx < fadeStartFrame) {
            continue;
        }

        uint64_t intoFade = idx - fadeStartFrame;
        double t = double(intoFade) / double(fadeFrames);

        if (t > 1.0) t = 1.0;

        double u = 1.0 - t;

        double gain = 
            u * u * u * 1.0
        +   3 * u * u * t * m_cp1.first
        +   3 * u * t * t * m_cp2.first
        +   t * t * t * 0.0;

        for (int c = 0; c < ch; ++c) {
            out[f * ch + c] *= static_cast<float>(gain);
        }
    }
}

std::string ClipFadeOut::name() const {
    return "FadeOut";
}


ClipLoop::ClipLoop(bool loop) 
    : m_loop(loop)
{}

void ClipLoop::apply(Clip& clip, float* out, unsigned long frames) {
    if (!m_loop) return;

    const uint64_t totalFrames = clip.frameCount();

    if (clip.pos >= totalFrames) {
        clip.pos = 0;
    }
}

std::string ClipLoop::name() const {
    return "Loop";
}


ClipGain::ClipGain(double gain)
    : m_gain(gain)
{}

void ClipGain::apply(Clip& clip, float* out, unsigned long frames) {
    for (unsigned long f = 0; f < frames; ++f) {
        for (int c = 0; c < clip.channels; ++c) {
            out[f * clip.channels + c] *= static_cast<float>(m_gain);
        }
    }
}

std::string ClipGain::name() const {
    return "Gain";
}