#pragma once

#include <cstdint>

class IAllocator
{
public:
    virtual ~IAllocator() = default;

    // 申请原始内存块
    virtual void *allocate(size_t size, size_t alignment = 8) = 0;

    // 释放原始内存块
    virtual void deallocate(void *ptr, size_t size) = 0;
};