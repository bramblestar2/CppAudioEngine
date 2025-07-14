#include "Audio/AudioManager.h"
#include <iostream>
#include <magic.h>

#ifndef MAGIC_DB_PATH
#define MAGIC_DB_PATH ""
#endif

AudioManager::AudioManager() {
}

void AudioManager::load(std::string file, int id) {
    magic_t magic = magic_open(MAGIC_MIME_TYPE | MAGIC_ERROR);
    const char* dbpath = MAGIC_DB_PATH[0] ? MAGIC_DB_PATH : NULL;
    if (!magic || magic_load(magic, dbpath) != 0) {
        magic_close(magic);
        return;
    }

    const char* result = magic_file(magic, file.c_str());
    std::string type(result);
    magic_close(magic);

    if (type.starts_with("audio/")) {
        std::cout << type << std::endl;
    }
}

void AudioManager::load(std::vector<float>& data, int channels, double sampleRate, int id) {
}

void AudioManager::load(AudioBuffer& audioBuffer, int id) {
}

void AudioManager::remove(int id) {
}

std::pair<int, AudioBuffer>* AudioManager::get(int index) const {
    return nullptr;
}

int AudioManager::size() const noexcept {
    return m_audioBuffers.size();
}