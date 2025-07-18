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
    
    // AudioBuffer audioBuffer = AudioBuffer::load("/home/jay/Music/powerUp.wav");
    AudioBuffer audioBufferTwo = AudioBuffer::load("/home/jay/Music/Pumpkin  - C418.mp3");

    // AudioBuffer audioBufferTwo = AudioBuffer::load("C:\\Users\\thega\\Music\\song.mp3");

    // AudioBuffer &audioBuffer = audioManager.get(0)->second;
    // AudioBuffer &audioBufferTwo = audioManager.get(1)->second;

    // Clip voice;
    // voice.channels = audioBuffer.channels;
    // voice.sampleRate = audioBuffer.sampleRate;
    // voice.data = audioBuffer.data.data();
    // voice.size = audioBuffer.data.size();

    // spdlog::info("BPM: {}", audioBufferTwo.estimatedBPM());

    Clip clip = Clip::fromBuffer(audioBufferTwo, 30000, 40000);
    clip.effects.push_back(std::make_shared<ClipFadeIn>(3.0));
    clip.effects.push_back(std::make_shared<ClipFadeOut>(3.0));
    clip.effects.push_back(std::make_shared<ClipLoop>());

    // audioEngine.addVoice(std::move(voice));
    audioEngine.addClip(clip);

    std::cin.get();

    audioEngine.stop();
    result = AudioEngine::shutdown();

    return 0;
}
