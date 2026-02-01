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
    ObjectFactory(PhysicalMemoryLayout mem);

    // 泛型对象创建：在物理内存上原位构造
    template <typename T, typename... Args>
    T *create(Args &&...args)
    {
        static_assert(!std::is_abstract_v<T>, "Cannot create abstract class with ObjectFactory");

        // 1. 调用已经写好的对齐分配逻辑
        // allocate_raw 会处理 _current_p 的对齐偏移，并扣除 _remaining
        void *mem = this->allocate_raw(sizeof(T));

        if (!mem)
            return nullptr;

        // 2. 在对齐后的内存上构造对象
        return new (mem) T(std::forward<Args>(args)...);
    }

    void deallocate(void *ptr)
    {
        // 什么也不做
    }

    // 申请一块裸内存（用于栈）
    void *ObjectFactory::allocate_raw(size_t size);
};