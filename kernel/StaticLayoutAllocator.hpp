#pragma once

#include <cstdint>
#include "IAllocator.hpp"
#include "KernelUtils.hpp"

#include "Memory.hpp"

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
    static StaticLayoutAllocator *create(PhysicalMemoryLayout &layout)
    {
        return new (layout.base) StaticLayoutAllocator(
            (uint8_t *)layout.base + sizeof(StaticLayoutAllocator),
            layout.size - sizeof(StaticLayoutAllocator));
    }

    StaticLayoutAllocator(void *base, size_t size)
        : _base(base), _size(size), _used(0) {}

    /** @brief 获取该分配器管理的总容量（不含分配器对象本身） */
    size_t get_capacity() const { return _size; }

    /** @brief 获取当前已使用的字节数（包含对齐填充） */
    size_t get_used_bytes() const { return _used; }

    /** @brief 获取当前理论剩余空间 */
    size_t get_free_size() const { return _size - _used; }

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