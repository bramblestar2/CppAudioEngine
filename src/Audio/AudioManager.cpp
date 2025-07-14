#include "Audio/AudioManager.h"
#include <iostream>
#include <algorithm>
#include <magic.h>
#include <sndfile.hh>

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
        SndfileHandle sf(file.c_str(), SFM_READ);

        int channels = sf.channels();
        double sampleRate = sf.samplerate();
        std::vector<float> data(sf.frames() * channels);
        sf.readf(data.data(), sf.frames());

        load(std::move(data), channels, sampleRate, id);
    }
}

void AudioManager::load(std::vector<float>&& data, int channels, double sampleRate, int id) {
    load(std::move(AudioBuffer(std::move(data), channels, sampleRate)), id);
}

void AudioManager::load(AudioBuffer&& audioBuffer, int id) {
    m_audioBuffers.push_back(std::make_pair(id, audioBuffer));
}

void AudioManager::remove_id(int id) {
    auto it = std::find_if(m_audioBuffers.begin(), m_audioBuffers.end(), [id](const std::pair<int, AudioBuffer>& pair) {
        return pair.first == id;
    });
    if (it != m_audioBuffers.end()) {
        m_audioBuffers.erase(it);
    }
}

void AudioManager::remove(int index) {
    if (index < 0 || index >= m_audioBuffers.size()) {
        return;
    }
    m_audioBuffers.erase(m_audioBuffers.begin() + index);
}

void AudioManager::clear() {
    m_audioBuffers.clear();
}

AudioBuffer* AudioManager::get_id(int id) const {
    return nullptr;
}

std::pair<int, AudioBuffer>* AudioManager::get(int index) const {
    return nullptr;
}

int AudioManager::size() const noexcept {
    return m_audioBuffers.size();
}