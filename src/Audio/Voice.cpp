#include "Audio/Voice.h"
#include "Audio/AudioBuffer.h"

Voice Voice::fromBuffer(AudioBuffer& buffer, uint64_t start /* ms */, uint64_t end /* ms */) {
    uint64_t totalFrames = buffer.frameCount();
    uint64_t startFrame = (start == 0) ? 0 : std::min(static_cast<uint64_t>(start * buffer.sampleRate / 1000), totalFrames);
    uint64_t endFrame = (end == 0) ? totalFrames : std::min(static_cast<uint64_t>(end * buffer.sampleRate / 1000), totalFrames);

    if (endFrame < startFrame) {
        endFrame = startFrame;
    }

    uint64_t frames = endFrame - startFrame;

    Voice voice;
    voice.channels = buffer.channels;
    voice.sampleRate = buffer.sampleRate;
    voice.data = buffer.data.data() + startFrame * voice.channels;
    voice.size = frames * voice.channels;
    return voice;
}
