#pragma once

class ClipEffect {
public:
    virtual void apply(class Clip& clip, float* data, unsigned long frames) = 0;
};


class ClipFadeIn : public ClipEffect {
public:
    void apply(class Clip& clip, float* data, unsigned long frames) override;
};

class ClipFadeOut : public ClipEffect {
public:
    void apply(class Clip& clip, float* data, unsigned long frames) override;
};