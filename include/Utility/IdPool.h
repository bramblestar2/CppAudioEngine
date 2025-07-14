#pragma once
#include <mutex>
#include <queue>

class IdPool {
public:
    IdPool();

    int acquire();
    bool reserve(int id);
    void release(int id);

    void clear();

private:
    std::mutex m_mutex;

    std::queue<int> m_freeIds;
    int m_nextId;
};