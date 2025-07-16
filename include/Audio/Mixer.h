#pragma once
#include "Audio/Voice.h"

class Mixer {
public:
    void addVoice(Voice&& voice);
    void removeVoice(int id);

    void process(float* out, unsigned long frames);
private:
};