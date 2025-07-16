#include "Audio/AudioEngine.h"
#include <spdlog/spdlog.h>

AudioEngine::AudioEngine()
    : m_stream(nullptr)
    , m_sampleRate(44100)
    , m_channels(2)
{
    
}

AudioEngine::~AudioEngine() {
    stop();
}


void AudioEngine::addVoice(Voice&& voice) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_voices.push_back(std::move(voice));
}

void AudioEngine::removeVoice(int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (id >= 0 && id < m_voices.size()) {
        m_voices.erase(m_voices.begin() + id);
    }   
}


bool AudioEngine::start() {
    if (m_stream != nullptr) {
        return true;
    }

    PaError err;

    PaStreamParameters sParams = {};
    sParams.device = Pa_GetDefaultOutputDevice();
    sParams.channelCount = m_channels;
    sParams.sampleFormat = paFloat32;
    sParams.suggestedLatency = Pa_GetDeviceInfo(sParams.device)->defaultLowOutputLatency;
    sParams.hostApiSpecificStreamInfo = nullptr;

    PaStreamFlags streamFlags = paClipOff;

    err = Pa_OpenStream(
        &m_stream,
        nullptr,
        &sParams,
        m_sampleRate, 
        paFramesPerBufferUnspecified,
        streamFlags,
        paCallback,
        this
    );

    if (err != paNoError) {
        spdlog::error("Failed to open PortAudio stream: {}", Pa_GetErrorText(err));
        return false;
    }

    err = Pa_StartStream(m_stream);

    if (err != paNoError) {
        spdlog::error("Failed to start PortAudio stream: {}", Pa_GetErrorText(err));
        Pa_CloseStream(m_stream);
        m_stream = nullptr;
        return false;
    }

    return true;
}

bool AudioEngine::stop() {
    if (m_stream != nullptr) {
        PaError err;
        err = Pa_StopStream(m_stream);
        if (err != paNoError) {
            spdlog::error("Failed to stop PortAudio stream: {}", Pa_GetErrorText(err));
        }
        err = Pa_CloseStream(m_stream);
        if (err != paNoError) {
            spdlog::error("Failed to close PortAudio stream: {}", Pa_GetErrorText(err));
        }
        m_stream = nullptr;
    }

    return true;
}


bool AudioEngine::init() {
    PaError err;
    err = Pa_Initialize();

    if (err != paNoError) {
        spdlog::error("Failed to initialize PortAudio: {}", Pa_GetErrorText(err));
        return false;
    }

    return true;
}

bool AudioEngine::shutdown() {
    PaError err;
    err = Pa_Terminate();

    if (err != paNoError) {
        spdlog::error("Failed to shutdown PortAudio: {}", Pa_GetErrorText(err));
        return false;
    }

    return true;
}

int AudioEngine::paCallback(const void* in, void* out, 
                            unsigned long frames, 
                            const PaStreamCallbackTimeInfo* timeInfo, 
                            PaStreamCallbackFlags flags, void* userData) {
    AudioEngine* audioEngine = reinterpret_cast<AudioEngine*>(userData);

    if (audioEngine == nullptr) {
        return paAbort;
    }

    return audioEngine->handleStreamCallback(in, out, frames, timeInfo, flags);
}


int AudioEngine::handleStreamCallback(
    const void* , void* outVoid,
    unsigned long frames,
    const PaStreamCallbackTimeInfo*,
    PaStreamCallbackFlags
) {
    auto* outF = static_cast<float*>(outVoid);
    const auto totalSamples = frames * m_channels;

    std::fill(outF, outF + totalSamples, 0.0f);

    std::lock_guard lock(m_mutex);

    for (auto& voice : m_voices) {
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
        return v.pos >= v.frameCount();
    });

    std::transform(outF, outF + totalSamples, outF,
                   [](float x){ return std::clamp(x, -1.0f, 1.0f); });

    return paContinue;
}