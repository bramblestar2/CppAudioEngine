#include "Utility/IdPool.h"
#include <stdexcept>

IdPool::IdPool()
    : m_nextId(1)
{
}

int IdPool::acquire() {
    std::lock_guard<std::mutex> lock(m_mutex);
    uint32_t id;
    if (!m_freeIds.empty()) {
        id = m_freeIds.top();
        m_freeIds.pop();
    } else {
        id = m_nextId;

        if (m_nextId == std::numeric_limits<uint32_t>::max()) {
            throw std::overflow_error("IdPool overflow");
        }
        
        ++m_nextId;
    }
    return id;
}

bool IdPool::reserve(uint32_t id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (id >= m_nextId) {
        if (id == std::numeric_limits<uint32_t>::max()) {
            throw std::overflow_error("IdPool overflow");
        }

        m_nextId = id + 1;

        return true;
    } else {
        std::vector<uint32_t> temp;
        bool found = false;

        while (!m_freeIds.empty()) {
            uint32_t x = m_freeIds.top();
            m_freeIds.pop();
            if (x == id) {
                found = true;
                continue;
            }
            temp.push_back(x);
        }

        for (auto x : temp) m_freeIds.push(x);

        return found;
    }
}

void IdPool::release(uint32_t id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (id < m_nextId) {
        bool exists = false;
        std::vector<uint32_t> temp;
        
        while (!m_freeIds.empty()) {
            uint32_t x = m_freeIds.top();
            m_freeIds.pop();
            if (x == id) exists = true;
            temp.push_back(x);
        }

        for (auto x : temp) m_freeIds.push(x);
        
        if (!exists) {
            m_freeIds.push(id);
        }
    }
}

bool IdPool::exists(uint32_t id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (id == 0 || id >= m_nextId) {
        return false;
    }

    std::vector<uint32_t> temp;
    bool inFree = false;

    while (!m_freeIds.empty()) {
        uint32_t x = m_freeIds.top();
        m_freeIds.pop();

        if (x == id) inFree = true;

        temp.push_back(x);
    }
    for (auto x : temp) m_freeIds.push(x);

    return !inFree;
}

void IdPool::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);

    while (!m_freeIds.empty()) m_freeIds.pop();
    
    m_nextId = 1;
}