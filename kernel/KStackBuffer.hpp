#pragma once

#include "KResource.hpp"
#include "KernelUtils.hpp"

class KStackBuffer : public KResource<uint8_t>
{
public:
    // 继承构造函数
    using KResource<uint8_t>::KResource;

    /**
     * 直接获取对齐后的栈顶
     * 封装了计算逻辑，Factory 不再需要知道栈是向上还是向下增长
     */
    void *get_aligned_top(size_t alignment = 16) const
    {
        if (!get())
            return nullptr;

        // 栈底地址 + 总大小 = 初始栈顶
        uintptr_t top = reinterpret_cast<uintptr_t>(get()) + size_in_bytes();

        // 执行对齐并返回
        return reinterpret_cast<void *>(KernelUtils::Align::down(top, alignment));
    }
};