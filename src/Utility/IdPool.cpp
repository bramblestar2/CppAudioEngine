#include "Utility/IdPool.h"

IdPool::IdPool()
    : m_nextId(0)
{
}

int IdPool::acquire() {

}

bool IdPool::reserve(int id) {
}

void IdPool::release(int id) {
}

void IdPool::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_freeIds = std::queue<int>();
    m_nextId = 0;
}