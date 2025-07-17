#include "Audio/Mixer.h"
#include <algorithm>
#include <spdlog/spdlog.h>

Mixer::Mixer(double sampleRate, int channels)
    : m_sampleRate(sampleRate)
    , m_channels(channels)
{
}

int Mixer::addVoice(Clip voice) {
    std::lock_guard<std::mutex> lock(m_mutex);
    int id = m_idPool.acquire();
    m_voices.push_back(std::make_pair(id, voice));
    return id;
}

void Mixer::removeVoice(int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::remove_if(m_voices.begin(), m_voices.end(), [id](const std::pair<int, Clip>& pair) {
        return pair.first == id;
    });
    
    m_voices.erase(it, m_voices.end());
}

void Mixer::process(float* out, unsigned long frames) {
    auto* outF = static_cast<float*>(out);
    const auto totalSamples = frames * m_channels;

    std::fill(outF, outF + totalSamples, 0.0f);

    std::lock_guard lock(m_mutex);

    for (auto& [id, voice] : m_voices) {
        const double step = voice.sampleRate / m_sampleRate;

        for (unsigned long f = 0; f < frames; ++f) {
            size_t idx = static_cast<size_t>(voice.pos);
            if (idx >= voice.frameCount()) {
                voice.pos = voice.frameCount();
                break;
            }

            double frac = voice.pos - idx;
            size_t next_idx = std::min(idx + 1, voice.frameCount() - 1);
            size_t base = idx * voice.channels;
            size_t next_base = next_idx * voice.channels;

            for (int ch = 0; ch < m_channels; ++ch) {
                int srcCh = std::min(ch, voice.channels - 1);
                float s0 = voice.data[base + srcCh];
                float s1 = voice.data[next_base + srcCh];
                float sample = s0 + (s1 - s0) * float(frac);
                outF[f * m_channels + ch] += sample;
            }

            voice.pos += step;
        }
    }

    std::erase_if(m_voices, [](auto& v){
        return v.second.pos >= v.second.frameCount();
    });

    std::transform(outF, outF + totalSamples, outF,
                   [](float x){ return std::clamp(x, -1.0f, 1.0f); });
}