#pragma once
#include "IAllocator.hpp"
#include "KernelUtils.hpp"

class KernelHeapAllocator : public IAllocator
{
private:
    friend class HeapInspector;
    struct HeapBlock
    {
        size_t size;
        bool is_used;
        HeapBlock *next;
    };

    void *_heap_start;
    size_t _heap_size;
    HeapBlock *_first_block;

public:
    /**
     * @param start 堆的起始线性地址
     * @param size  堆的总大小
     */
    KernelHeapAllocator(void *start, size_t size)
        : _heap_start(start), _heap_size(size)
    {

        // 初始化第一个大空闲块
        _first_block = reinterpret_cast<HeapBlock *>(start);
        _first_block->size = size;
        _first_block->is_used = false;
        _first_block->next = nullptr;
    }

    void *allocate(size_t size, size_t alignment = 8) override
    {
        // 1. 计算实际需要的尺寸：请求大小 + Header 大小，并对齐
        size_t total_needed = KernelUtils::Align::up(size + sizeof(HeapBlock), alignment);

        HeapBlock *curr = _first_block;
        while (curr)
        {
            // 2. 查找足够大的空闲块 (First Fit 策略)
            if (!curr->is_used && curr->size >= total_needed)
            {

                // 3. 检查是否可以“切分”出剩余空间，避免浪费
                // 如果剩余空间连一个 Header 都放不下，就不切分了
                if (curr->size >= total_needed + sizeof(HeapBlock) + 8)
                {
                    HeapBlock *next_block = reinterpret_cast<HeapBlock *>(
                        reinterpret_cast<uintptr_t>(curr) + total_needed);
                    next_block->size = curr->size - total_needed;
                    next_block->is_used = false;
                    next_block->next = curr->next;

                    curr->size = total_needed;
                    curr->next = next_block;
                }

                curr->is_used = true;
                // 返回 Header 之后的有效载荷地址
                return reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(curr) + sizeof(HeapBlock));
            }
            curr = curr->next;
        }

        return nullptr; // 内存耗尽
    }

    void deallocate(void *ptr, size_t size) override
    {
        if (!ptr)
            return;

        // 1. 根据指针回推 Header 地址
        HeapBlock *block = reinterpret_cast<HeapBlock *>(
            reinterpret_cast<uintptr_t>(ptr) - sizeof(HeapBlock));
        block->is_used = false;

        // 2. 简单的碎片合并 (Coalescing)
        // 遍历并尝试合并相邻的空闲块
        HeapBlock *curr = _first_block;
        while (curr && curr->next)
        {
            if (!curr->is_used && !curr->next->is_used)
            {
                curr->size += curr->next->size;
                curr->next = curr->next->next;
                // 合并后不移动 curr，继续尝试与新的 next 合并
            }
            else
            {
                curr = curr->next;
            }
        }
    }
};