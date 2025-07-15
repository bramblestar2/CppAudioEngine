#include <iostream>
#include <spdlog/spdlog.h>
#include "Audio/AudioEngine.h"

int main()
{
    bool result = AudioEngine::init();

    AudioEngine audioEngine;
    audioEngine.start();

    AudioManager audioManager;
    int id = audioManager.load("/home/jay/Music/powerUp.wav");
    
    AudioBuffer &audioBuffer = audioManager.get(0)->second;

    Voice voice;
    voice.channels = audioBuffer.channels;
    voice.sampleRate = audioBuffer.sampleRate;
    voice.data = audioBuffer.data.data();
    voice.size = audioBuffer.data.size() / 2;

    spdlog::info("{} | {} | {} | {}", voice.frameCount(), voice.size, voice.channels, voice.sampleRate);

    audioEngine.addVoice(std::move(voice));

    std::cin.get();
    
    audioEngine.stop();
    result = AudioEngine::shutdown();

    return 0;
}
