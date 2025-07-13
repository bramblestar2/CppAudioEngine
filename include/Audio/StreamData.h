#pragma once

struct StreamData {
    int channels;
    double sampleRate;
    unsigned long sampleSize;
    
    double bpm;
    unsigned long framesPerBeat;
    unsigned long frameCounter;
};