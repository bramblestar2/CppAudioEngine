#include "Audio/AudioBuffer.h"
#include <aubio/aubio.h>
#include <numeric>
#include <string>
#include <spdlog/spdlog.h>
#include <sndfile.hh>


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

AudioBuffer AudioBuffer::load(std::string file, uint64_t start /* ms */, uint64_t end /* ms */, double sampleRate) {
    SndfileHandle sndfile(file);

    bool resample = (sampleRate > 0 && sampleRate != sndfile.samplerate());

    if (sampleRate == -1) {
        sampleRate = sndfile.samplerate();
    } else {
        resample = true;
    }

    int channels = sndfile.channels();

    if (start == 0 && end == 0) {
        std::vector<float> data(sndfile.frames() * channels);
        sndfile.readf(data.data(), sndfile.frames());

        if (resample) {
            data = resampleBuffer(data, sndfile.samplerate(), sampleRate, channels);
        }

        return AudioBuffer(std::move(data), channels, sampleRate);
    }


    uint64_t totalFrames = sndfile.frames();
    uint64_t startFrame = std::min(static_cast<uint64_t>(start * sndfile.samplerate() / 1000), totalFrames);
    uint64_t endFrame = std::min(static_cast<uint64_t>(end * sndfile.samplerate() / 1000), totalFrames);
    if (endFrame < startFrame) {
        endFrame = startFrame;
    }
    int countFrames = endFrame - startFrame;

    std::vector<float> data(countFrames * channels);
    sndfile.seek(static_cast<int64_t>(startFrame), SEEK_SET);
    sndfile.readf(data.data(), countFrames);

    if (resample) {
        data = resampleBuffer(data, sndfile.samplerate(), sampleRate, channels);
    }
    
    return AudioBuffer(std::move(data), channels, sampleRate);
}




std::vector<float> resampleBuffer(std::vector<float>& dataVec, double inputSampleRate, double outputSampleRate, int channels, int converterType) {
    if (channels < 1)
        throw std::invalid_argument("channels must be >= 1");

    size_t inFrames = dataVec.size() / channels;
    if (inFrames * channels != dataVec.size())
        throw std::invalid_argument("Input size not divisible by channels");

    double ratio = outputSampleRate / inputSampleRate;
    size_t maxOutFrames = static_cast<size_t>(inFrames * ratio) + 1;

    std::vector<float> out(maxOutFrames * channels);

    SRC_DATA data;
    data.data_in        = dataVec.data();
    data.input_frames   = static_cast<long>(inFrames);
    data.data_out       = out.data();
    data.output_frames  = static_cast<long>(maxOutFrames);
    data.src_ratio      = ratio;
    data.end_of_input   = 1;
    data.input_frames_used = 0;
    data.output_frames_gen = 0;

    int err = src_simple(&data, converterType, channels);
    if (err != 0) {
        throw std::runtime_error(std::string("libsamplerate error: ")
                                 + src_strerror(err));
    }

    out.resize(data.output_frames_gen * channels);
    return out;
}