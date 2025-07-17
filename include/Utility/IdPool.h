#pragma once
#include <mutex>
#include <queue>
#include <vector>
#include <cstdint>
#include <functional>

class IdPool {
public:
    IdPool();

    int acquire();
    bool reserve(uint32_t id);
    void release(uint32_t id);

    bool exists(uint32_t id);

    void clear();

private:
    std::mutex m_mutex;

    std::priority_queue<
        uint32_t, 
        std::vector<uint32_t>, 
        std::greater<uint32_t>> m_freeIds;
    int m_nextId;
};