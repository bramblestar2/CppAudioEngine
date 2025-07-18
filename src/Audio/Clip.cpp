#include "Audio/Clip.h"
#include "Audio/AudioBuffer.h"

Clip Clip::fromBuffer(AudioBuffer& buffer, uint64_t start /* ms */, uint64_t end /* ms */) {
    uint64_t totalFrames = buffer.frameCount();
    uint64_t startFrame = (start == 0) ? 0 : std::min(static_cast<uint64_t>(start * buffer.sampleRate / 1000), totalFrames);
    uint64_t endFrame = (end == 0) ? totalFrames : std::min(static_cast<uint64_t>(end * buffer.sampleRate / 1000), totalFrames);

    if (endFrame < startFrame) {
        endFrame = startFrame;
    }

    uint64_t frames = endFrame - startFrame;

    Clip clip;
    clip.channels = buffer.channels;
    clip.sampleRate = buffer.sampleRate;
    clip.data = buffer.data.data() + startFrame * clip.channels;
    clip.size = frames * clip.channels;
    // clip.bpm = buffer.estimatedBPM();
    return clip;
}
