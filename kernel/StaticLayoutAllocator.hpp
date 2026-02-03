#pragma once

#include <cstdint>
#include "IAllocator.hpp"
#include "KernelUtils.hpp"

/**
 * StaticLayoutAllocator: 管理一块预先确定的内存布局
 */
class StaticLayoutAllocator : public IAllocator
{
private:
    void *_base;
    size_t _size;
    size_t _used;

public:
    StaticLayoutAllocator(void *base, size_t size)
        : _base(base), _size(size), _used(0) {}

    void *allocate(size_t size, size_t alignment = 8) override
    {
        // 使用我们之前定义的 KernelUtils 进行对齐计算
        uintptr_t current_pos = (uintptr_t)_base + _used;
        uintptr_t aligned_pos = KernelUtils::Align::up(current_pos, alignment);
        size_t padding = aligned_pos - current_pos;

        if (_used + padding + size > _size)
        {
            return nullptr; // 内存溢出
        }

        _used += padding + size;
        return (void *)aligned_pos;
    }

    void deallocate(void *ptr, size_t size) override
    {
        // 静态布局分配器通常不支持随机释放（类似线性分配器）
        // 如果需要随机释放，则需要内部实现空闲链表或位图
    }
};