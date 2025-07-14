#include <portaudio.h>
#include <iostream>

#include "Audio/AudioManager.h"

static int paCallback(const void* in, void* out, unsigned long frames, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*) {
    float* output = static_cast<float*>(out);
    unsigned long totalSamples = frames; // mono
    for(unsigned long i = 0; i < totalSamples; ++i) {
        output[i] = 0.0f;
    }
    
    return paContinue;
}

int main()
{
    AudioManager audioManager;
    audioManager.load("/home/jay/Music/powerUp.wav");

    PaError err;

    // 1) Initialize
    err = Pa_Initialize();
    if( err != paNoError ) {
        std::cerr << "Pa_Initialize failed: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    // 2) Choose a host API that isn’t JACK (optional)
    //    If you want to force ALSA directly, uncomment below:
    /*
    int numAPIs = Pa_GetHostApiCount();
    for(int i=0; i<numAPIs; ++i){
      const PaHostApiInfo* info = Pa_GetHostApiInfo(i);
      if(info->type == paALSA){
        Pa_Initialize(); // already done
        Pa_Terminate();  // hack: you'd actually open with explicit host API index
      }
    }
    */

    std::cout << "Device count: " << Pa_GetDeviceCount() << "\n";

    // 3) Open a stream with the callback
    PaStream* stream = nullptr;
    err = Pa_OpenDefaultStream(
        &stream,
        0,              // no input channels
        1,              // mono output
        paFloat32,      // 32‑bit floating point output
        44100,          // sample rate
        256,            // frames per buffer
        paCallback,     // your callback
        nullptr         // user data
    );
    if( err != paNoError ) {
        std::cerr << "Pa_OpenDefaultStream failed: " << Pa_GetErrorText(err) << "\n";
        Pa_Terminate();
        return 1;
    }

    // 4) Start the stream
    err = Pa_StartStream(stream);
    if( err != paNoError ) {
        std::cerr << "Pa_StartStream failed: " << Pa_GetErrorText(err) << "\n";
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    std::cout << "PortAudio version: " << Pa_GetVersionText() << "\n";
    std::cout << "Stream is running — press Enter to stop\n";
    std::cin.get();

    // 5) Stop and clean up
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
