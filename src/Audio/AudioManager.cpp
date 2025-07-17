#include "Audio/AudioManager.h"
#include <iostream>
#include <algorithm>
#include <magic.h>
#include <sndfile.hh>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <sqlite3.h>

#ifndef MAGIC_DB_PATH
#define MAGIC_DB_PATH ""
#endif

AudioManager::AudioManager() {
}

int AudioManager::load(std::string file, int id) {
    if (id == -1) {
        id = m_idPool.acquire();
    } else if (m_idPool.exists(id)) {
        return -1;
    } else {
        m_idPool.reserve(id);
    }

    if (!std::filesystem::exists(file)) {
        spdlog::error("File does not exist: {}", file);
        return -1;
    }

    magic_t magic = magic_open(MAGIC_MIME_TYPE | MAGIC_ERROR);
    const char* dbpath = MAGIC_DB_PATH[0] ? MAGIC_DB_PATH : NULL;
    if (!magic || magic_load(magic, dbpath) != 0) {
        magic_close(magic);
        spdlog::error("Failed to open magic database");
        return -1;
    }

    const char* result = magic_file(magic, file.c_str());
    std::string type(result);
    magic_close(magic);

    if (type.starts_with("audio/")) {
        AudioBuffer audioBuffer = AudioBuffer::load(file);
        return load(std::move(audioBuffer), id);
    }

    return -1;
}

int AudioManager::load(std::vector<float>&& data, int channels, double sampleRate, int id) {
    if (id == -1) {
        id = m_idPool.acquire();
    } else if (m_idPool.exists(id)) {
        return -1;
    } else {
        m_idPool.reserve(id);
    }

    return load(std::move(AudioBuffer(std::move(data), channels, sampleRate)), id);
}

int AudioManager::load(AudioBuffer&& audioBuffer, int id) {
    if (id == -1) {
        id = m_idPool.acquire();
    } else if (m_idPool.exists(id)) {
        return -1;
    } else {
        m_idPool.reserve(id);
    }

    m_audioBuffers.push_back(std::make_pair(id, audioBuffer));
    return id;
}

void AudioManager::remove_id(int id) {
    auto it = std::find_if(m_audioBuffers.begin(), m_audioBuffers.end(), [id](const std::pair<int, AudioBuffer>& pair) {
        return pair.first == id;
    });
    if (it != m_audioBuffers.end()) {
        m_idPool.release(id);
        m_audioBuffers.erase(it);
    }
}

void AudioManager::remove(int index) {
    if (index < 0 || index >= m_audioBuffers.size()) {
        return;
    }
    m_idPool.release(m_audioBuffers[index].first);
    m_audioBuffers.erase(m_audioBuffers.begin() + index);
}

void AudioManager::clear() {
    m_audioBuffers.clear();
    m_idPool.clear();
}

AudioBuffer* AudioManager::get_id(int id) {
    auto it = std::find_if(m_audioBuffers.begin(), m_audioBuffers.end(), [id](std::pair<int, AudioBuffer>& pair) {
        return pair.first == id;
    });

    if (it != m_audioBuffers.end()) {
        return &it->second;
    }

    return nullptr;
}

std::pair<int, AudioBuffer>* AudioManager::get(int index) {
    if (index >= 0 && index < m_audioBuffers.size()) {
        return &m_audioBuffers[index];
    }

    return nullptr;
}

int AudioManager::size() const noexcept {
    return m_audioBuffers.size();
}

bool AudioManager::save(std::string dbPath) {
    if (!std::filesystem::exists(dbPath)) {
        spdlog::error("Database does not exist: {}", dbPath);
        return false;
    }

    sqlite3* db = nullptr;
    char* err = nullptr;

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        spdlog::error("Failed to open database: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    if (sqlite3_exec(db, "BEGIN;", nullptr, nullptr, &err) != SQLITE_OK) {
        spdlog::error("Failed to begin transaction: {}", err);
        sqlite3_free(err);
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);

    return true;
}

bool AudioManager::open(std::string db) {
    if (!std::filesystem::exists(db)) {
        spdlog::error("Database does not exist: {}", db);
        return false;
    }

    return true;
}