#include "Audio/AudioBuffer.h"
#include <aubio/aubio.h>
#include <numeric>
#include <string>
#include <spdlog/spdlog.h>
#include <sndfile.hh>
#include <samplerate.h>


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


AudioBuffer AudioBuffer::load(std::string file) {
    SndfileHandle sndfile(file);

    int channels = sndfile.channels();
    double sampleRate = sndfile.samplerate();
    std::vector<float> data(sndfile.frames() * channels);
    sndfile.readf(data.data(), sndfile.frames());

    return AudioBuffer(std::move(data), channels, sampleRate);
}

AudioBuffer AudioBuffer::load(std::string file, double start /*seconds*/, double end /*seconds*/, double sampleRate, int channels) {
    SndfileHandle sndfile(file);

    bool resample = false;

    if (sampleRate == -1) {
        sampleRate = sndfile.samplerate();
    } else {
        resample = true;
    }

    if (channels == -1) {
        channels = sndfile.channels();
    } else {
        resample = true;
    }

    auto resample = [&](std::vector<float>& data, int channels, double sampleRate) {
        int error;
        SRC_STATE *state = src_new(SRC_SINC_MEDIUM_QUALITY, channels, &error);
        if (!state) {
            spdlog::error("Failed to create SRC_STATE: {}", error);
            return;
        }

        double ratio = sampleRate / sndfile.samplerate();

        int INPUT_FRAMES = 512;
        int OUTPUT_FRAMES = (int)(INPUT_FRAMES * ratio + 1);

        float in[INPUT_FRAMES * channels];
        float out[OUTPUT_FRAMES * channels];
        SRC_DATA srcData;
        srcData.data_in = in;
        srcData.data_out = out;
        srcData.input_frames = INPUT_FRAMES;
        srcData.output_frames = OUTPUT_FRAMES;
        srcData.src_ratio = ratio;
        srcData.end_of_input = 1;
        srcData.input_frames_used = 0;
        srcData.output_frames_gen = 0;
        
        
    };

    if (start == 0 && end == 0) {
        std::vector<float> data(sndfile.frames() * channels);
        sndfile.readf(data.data(), sndfile.frames());
        return AudioBuffer(std::move(data), channels, sampleRate);
    }

    int startFrame = (int)(std::min(start, end) * sampleRate);
    int endFrame = (int)(std::max(start, end) * sampleRate);
    int frames = endFrame - startFrame;

    if (startFrame < 0) {
        startFrame = 0;
    }

    if (endFrame > sndfile.frames()) {
        endFrame = sndfile.frames();
    }

    if (startFrame > endFrame) {
        startFrame = endFrame;
    }

    std::vector<float> data((end - start) * channels);
    sndfile.seek(sndfile.frames() * start, SEEK_SET);
    sndfile.readf(data.data(), frames);
    return AudioBuffer(std::move(data), channels, sampleRate);
}