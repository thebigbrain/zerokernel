#pragma once

#include <utility>
#include <new>

#include "IAllocator.hpp"
#include <common/PlacementNew.hpp>

class IObjectBuilder
{
protected:
    IAllocator *_allocator;

    virtual void on_object_created() = 0;

public:
    IObjectBuilder(IAllocator *alloc) : _allocator(alloc) {}
    virtual ~IObjectBuilder() = default;

    /**
     * 核心能力：在 Allocator 提供的空间上构建对象
     */
    template <typename T, typename... Args>
    T *construct(Args &&...args)
    {
        // 1. 调用 Allocator 分配空间
        void *ptr = _allocator->allocate(sizeof(T), alignof(T));
        if (!ptr)
            return nullptr;

        // 2. 执行 Placement New (Builder 的核心职责)
        return new (ptr) T(std::forward<Args>(args)...);
    }

    /**
     * 销毁对象并归还内存
     */
    template <typename T>
    void destroy(T *ptr)
    {
        if (!ptr)
            return;

        // 1. 显式调用析构函数
        ptr->~T();

        // 2. 归还内存给 Allocator
        _allocator->deallocate(ptr, sizeof(T));
    }
};