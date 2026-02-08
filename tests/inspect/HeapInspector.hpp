#pragma once

#include <kernel/KernelHeapAllocator.hpp>

/**
 * @brief 堆状态检查员（实例对象版）
 */
class HeapInspector
{
private:
    KernelHeapAllocator *_target;

public:
    /**
     * @brief 绑定一个需要透视的分配器
     */
    explicit HeapInspector(IAllocator *alloc)
    {
        // 将基类指针强转为友元类识别的具体实现类
        _target = static_cast<KernelHeapAllocator *>(alloc);
    }

    /**
     * @brief 实时遍历链表，统计总剩余空间
     */
    size_t get_free_size() const
    {
        if (!_target)
            return 0;

        size_t total_free = 0;
        auto *curr = _target->_first_block; // 直接访问私有成员

        while (curr)
        {
            if (!curr->is_used)
            {
                total_free += curr->size;
            }
            curr = curr->next;
        }
        return total_free;
    }

    /**
     * @brief 统计当前已分配空间
     */
    size_t get_used_size() const
    {
        if (!_target)
            return 0;

        size_t total_used = 0;
        auto *curr = _target->_first_block;

        while (curr)
        {
            if (curr->is_used)
            {
                total_used += curr->size;
            }
            curr = curr->next;
        }
        return total_used;
    }

    /**
     * @brief 获取链表中的块总数（用于分析碎片化程度）
     */
    size_t get_block_count() const
    {
        size_t count = 0;
        auto *curr = _target->_first_block;
        while (curr)
        {
            count++;
            curr = curr->next;
        }
        return count;
    }
};