#include "Audio/AudioEngine.h"
#include <spdlog/spdlog.h>
#include <algorithm>

AudioEngine::AudioEngine()
    : m_stream(nullptr)
    , m_sampleRate(44100)
    , m_channels(2)
    , m_mixer(m_sampleRate, m_channels)
{
    
}

AudioEngine::~AudioEngine() {
    stop();
}


int AudioEngine::addClip(Clip clip) {
    return m_mixer.addClip(clip);
}

void AudioEngine::removeClip(int id) {
    m_mixer.removeClip(id);
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

    std::fill(outF, outF + frames * m_channels, 0.0f);

    m_mixer.process(outF, frames);

    return paContinue;
}