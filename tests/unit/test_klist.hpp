// unit/test_klist.hpp
#pragma once

#include "test_framework.hpp"

#include "kernel/KList.hpp"
#include "kernel/IObjectBuilder.hpp"
#include <kernel/StaticLayoutAllocator.hpp>
#include <kernel/KernelObjectBuilder.hpp>

inline void unit_test_klist_allocation()
{
    // 使用本地栈分配器模拟，排除全局堆的影响
    uint8_t scratch[1024];
    StaticLayoutAllocator loader(scratch, 1024);
    KernelObjectBuilder builder(&loader);

    // 测试场景 1：存储原始指针（最常见的内核场景）
    KList<void *> list(&builder);

    // 触发点：第一次分配 ListNode<void*>
    // 如果 StaticLayoutAllocator 分配出的地址不是 8 字节对齐，这里必崩
    list.push_back((void *)0x12345678);

    K_T_ASSERT(!list.empty(), "List should not be empty");
    K_T_ASSERT(list.front() == (void *)0x12345678, "Data mismatch");

    // 测试场景 2：测试 Iterator 是否正常工作（验证内存连续访问）
    int count = 0;
    for (auto item : list)
    {
        if (item == (void *)0x12345678)
            count++;
    }
    K_T_ASSERT(count == 1, "Iterator traversal failed");

    // 测试场景 3：测试 remove_match (涉及 destroy 回收)
    list.remove_match([](void *p)
                      { return p == (void *)0x12345678; });
    K_T_ASSERT(list.empty(), "List clear failed via remove_match");
}