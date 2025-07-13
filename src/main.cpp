#include <portaudio.h>
#include <iostream>

static int paCallback(const void* in, void* out, unsigned long frames, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*) {
    const float* input = (const float*)in;
    float* output = (float*)out;
    for (unsigned long i = 0; i < frames; ++i) {
        output[i] = input[i];
    }
    
    return paContinue;
}

int main() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        return err;
    }

    PaStream* stream = nullptr;
    PaStreamParameters sParams = {};
    sParams.device = Pa_GetDefaultOutputDevice();
    sParams.channelCount = 1;
    sParams.sampleFormat = paFloat32;
    sParams.suggestedLatency = Pa_GetDeviceInfo(sParams.device)->defaultLowOutputLatency;
    sParams.hostApiSpecificStreamInfo = nullptr;

    Pa_OpenDefaultStream(
        &stream,
        1, 1,
        paFloat32,
        44100, 256,
        paCallback,
        nullptr
    );

    Pa_StartStream(stream);

    std::cout << "PortAudio version: " << Pa_GetVersionText() << std::endl;

    std::cin.get();


    Pa_StopStream(stream);
    Pa_CloseStream(stream);

    Pa_Terminate();

    return 0;
}