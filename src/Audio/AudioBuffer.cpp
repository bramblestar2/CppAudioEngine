#include "Audio/AudioBuffer.h"
#include <aubio/aubio.h>
#include <numeric>


AudioBuffer::AudioBuffer(std::vector<float>&& data, int channels, double sampleRate)
    : data(std::move(data))
    , channels(channels)
    , sampleRate(sampleRate)
    , beatsPerMinute(0)
{
}

float AudioBuffer::estimatedBPM() {
    return beatsPerMinute;
}

void AudioBuffer::calculateBPM() {
    if (beatsPerMinute == 0) {
        return;
    }

    const uint_t win_size = 1024;
    const uint_t hop_size = 512;

    aubio_tempo_t* tempo = new_aubio_tempo("default", win_size, hop_size, (uint_t)sampleRate);
    if (!tempo) {
        return;
    }

    fvec_t* in = new_fvec(hop_size);
    fvec_t* out = new_fvec(2);

    std::vector<float> bpmEstimates;

    size_t totalFrames = data.size() / channels;
    size_t readPos = 0;

    while (readPos + hop_size <= totalFrames) {
        for (size_t i = 0; i < hop_size; ++i) {
            in->data[i] = data[(readPos + i) * channels];
        }

        aubio_tempo_do(tempo, in, out);

        if (out->data[1] > 0) {
            bpmEstimates.push_back(out->data[1]);
        }

        readPos += hop_size;
    }

    del_aubio_tempo(tempo);
    del_fvec(in);
    del_fvec(out);

    if (!bpmEstimates.empty()) {
        float sum = std::accumulate(bpmEstimates.begin(), bpmEstimates.end(), 0.0f);
        beatsPerMinute = sum / bpmEstimates.size();
    } else {
        beatsPerMinute = 0;
    }
}