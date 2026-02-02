#pragma once
#include <common/Message.hpp>

// 依然需要这个底层函数指针定义
typedef void (*KernelMessageCallbackFunc)(const Message &, void *);

struct KernelCallback
{
    KernelMessageCallbackFunc func;
    void *context;

    // 默认构造
    KernelCallback() : func(nullptr), context(nullptr) {}

    // 方便构造
    KernelCallback(KernelMessageCallbackFunc f, void *ctx) : func(f), context(ctx) {}

    // 静态辅助方法：不分配内存，只返回一个值对象
    static KernelCallback Create(KernelMessageCallbackFunc f, void *ctx = nullptr)
    {
        return KernelCallback(f, ctx);
    }

    // 执行方法
    void invoke(const Message &msg) const
    {
        if (func)
        {
            func(msg, context);
        }
    }

    // 判断是否有效
    bool is_valid() const { return func != nullptr; }
};