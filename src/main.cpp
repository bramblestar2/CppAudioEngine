#include <iostream>
#include <spdlog/spdlog.h>
#include "Audio/AudioEngine.h"

int main()
{
    bool result = AudioEngine::init();

    AudioEngine audioEngine;
    audioEngine.start();

    AudioManager audioManager;
    // int id = audioManager.load("/home/jay/Music/Pumpkin  - C418.mp3");
    // int idTwo = audioManager.load("/home/jay/Music/laserShoot.wav");
    
    AudioBuffer audioBuffer = AudioBuffer::load("/home/jay/Music/powerUp.wav");
    AudioBuffer audioBufferTwo = AudioBuffer::load("/home/jay/Music/Pumpkin  - C418.mp3");

    // AudioBuffer &audioBuffer = audioManager.get(0)->second;
    // AudioBuffer &audioBufferTwo = audioManager.get(1)->second;

    Voice voice;
    voice.channels = audioBuffer.channels;
    voice.sampleRate = audioBuffer.sampleRate;
    voice.data = audioBuffer.data.data();
    voice.size = audioBuffer.data.size();

    Voice Vtwo = Voice::fromBuffer(audioBufferTwo, 30000, 40000);

    spdlog::info("{} | {} | {} | {}", voice.frameCount(), voice.size, voice.channels, voice.sampleRate);

    audioEngine.addVoice(std::move(voice));
    audioEngine.addVoice(std::move(Vtwo));

    std::cin.get();
    
    audioEngine.stop();
    result = AudioEngine::shutdown();

    return 0;
}
