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
    sParams.channelCount = 1;
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
    const void* in, void* out, 
    unsigned long frames, 
    const PaStreamCallbackTimeInfo* timeInfo, 
    PaStreamCallbackFlags flags
) {
    float* outF = static_cast<float*>(out);
    unsigned long numSamples = frames * m_channels;
    
    // Zero the buffer safely
    std::fill(outF, outF + numSamples, 0.0f);

    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto it = m_voices.begin(); it != m_voices.end();) {
        Voice& voice = *it;
        double increment = voice.sampleRate / m_sampleRate;
        bool voiceFinished = false;

        for (unsigned long f = 0; f < frames; ++f) {
            size_t idx = static_cast<size_t>(voice.pos);
            
            if (idx >= voice.frameCount()) {
                voiceFinished = true;
                break;
            }

            double frac = voice.pos - idx;
            size_t next_idx = (idx + 1 < voice.frameCount()) ? idx + 1 : idx;

            if (voice.channels == 1) {
                float sample = interpolate(voice, idx, next_idx, frac, 0);
                for (int ch = 0; ch < m_channels; ++ch) {
                    outF[f * m_channels + ch] += sample;
                }
            }
            else {
                for (int ch = 0; ch < m_channels; ++ch) {
                    int srcCh = std::min(ch, voice.channels - 1);
                    float sample = interpolate(voice, idx, next_idx, frac, srcCh);
                    outF[f * m_channels + ch] += sample;
                }
            }

            voice.pos += increment;
            if (voice.pos >= voice.frameCount()) {
                voiceFinished = true;
                break;
            }
        }

        if (voiceFinished) {
            it = m_voices.erase(it);
        } else {
            ++it;
        }
    }

    // Apply clipping prevention
    for (unsigned long i = 0; i < numSamples; ++i) {
        outF[i] = std::clamp(outF[i], -1.0f, 1.0f);
    }

    return paContinue;
}

float AudioEngine::interpolate(const Voice& voice, size_t idx, size_t next_idx, double frac, int channel) {
    size_t base_idx = idx * voice.channels + channel;
    // Ensure we don't access beyond the buffer
    size_t next_base_idx = next_idx * voice.channels + channel;
    
    float s0 = voice.data[base_idx];
    float s1 = (next_base_idx < voice.frameCount() * voice.channels) 
               ? voice.data[next_base_idx] 
               : s0;  // Use current sample if next is out of bounds
    
    return static_cast<float>(s0 + (s1 - s0) * frac);
}