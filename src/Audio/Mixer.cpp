#include "Audio/Mixer.h"
#include <algorithm>
#include <spdlog/spdlog.h>

Mixer::Mixer(double sampleRate, int channels)
    : m_sampleRate(sampleRate)
    , m_channels(channels)
{
}

int Mixer::addClip(Clip clip) {
    std::lock_guard<std::mutex> lock(m_mutex);
    int id = m_idPool.acquire();
    m_clips.push_back(std::make_pair(id, clip));
    return id;
}

void Mixer::removeClip(int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::remove_if(m_clips.begin(), m_clips.end(), [id](const std::pair<int, Clip>& pair) {
        return pair.first == id;
    });
    
    m_clips.erase(it, m_clips.end());
}

void Mixer::process(float* out, unsigned long frames) {
    auto* outF = static_cast<float*>(out);
    const auto totalSamples = frames * m_channels;

    std::fill(outF, outF + totalSamples, 0.0f);

    std::lock_guard lock(m_mutex);

    for (auto& [id, clip] : m_clips) {
        const double step = (clip.sampleRate / m_sampleRate);

        std::vector<float> tmp(totalSamples, 0.0f);

        for (unsigned long f = 0; f < frames; ++f) {
            size_t idx = static_cast<size_t>(clip.pos);
            if (idx >= clip.frameCount()) {
                clip.pos = clip.frameCount();
                break;
            }

            double frac = clip.pos - idx;
            size_t next_idx = std::min(idx + 1, clip.frameCount() - 1);
            size_t base = idx * clip.channels;
            size_t next_base = next_idx * clip.channels;

            for (int ch = 0; ch < m_channels; ++ch) {
                int srcCh = std::min(ch, clip.channels - 1);
                float s0 = clip.data[base + srcCh];
                float s1 = clip.data[next_base + srcCh];
                float sample = s0 + (s1 - s0) * float(frac);
                tmp[f * m_channels + ch] += sample;
            }

            clip.pos += step;
        }

        for (auto& eff : clip.effects) {
            eff->apply(clip, tmp.data(), frames);
        }

        for (size_t i = 0; i < totalSamples; ++i) {
            outF[i] += tmp[i];
        }
    }

    std::erase_if(m_clips, [](auto& v){
        return v.second.pos >= v.second.frameCount();
    });

    std::transform(outF, outF + totalSamples, outF,
                   [](float x){ return std::clamp(x, -1.0f, 1.0f); });
}