#pragma once
#include <cstdint>
#include <utility>
#include <new>
#include "Memory.hpp"

class ObjectFactory
{
private:
    uint8_t *_current_p;
    size_t _remaining;

public:
    ObjectFactory(PhysicalMemoryLayout mem)
        : _current_p((uint8_t *)mem.base), _remaining(mem.size) {}

    // 泛型对象创建：在物理内存上原位构造
    template <typename T, typename... Args>
    T *create(Args &&...args)
    {
        static_assert(!std::is_abstract_v<T>, "Cannot create abstract class with ObjectFactory");

        if (_remaining < sizeof(T))
            return nullptr;

        T *obj = new (_current_p) T(std::forward<Args>(args)...);
        _current_p += sizeof(T);
        _remaining -= sizeof(T);
        return obj;
    }

    // 申请一块裸内存（用于栈）
    void *allocate_raw(size_t size)
    {
        if (_remaining < size)
            return nullptr;
        void *ptr = _current_p;
        _current_p += size;
        _remaining -= size;
        return (uint8_t *)ptr; // 返回栈顶
    }
};