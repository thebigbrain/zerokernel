#pragma once
#include <common/Message.hpp>

// 这个宏的作用是：自动生成一个静态的“跳板”并打包成 MessageCallback 对象
#define BIND_MESSAGE_CB(Class, Func, ObjPtr)                                     \
    MessageCallback(                                                             \
        [](const Message &m, void *ctx) { static_cast<Class *>(ctx)->Func(m); }, \
        static_cast<void *>(ObjPtr))

// 依然需要这个底层函数指针定义
typedef void (*KernelMessageCallbackFunc)(const Message &, void *);

struct MessageCallback
{
    KernelMessageCallbackFunc func;
    void *context;

    // 默认构造
    MessageCallback() : func(nullptr), context(nullptr) {}

    // 方便构造
    MessageCallback(KernelMessageCallbackFunc f, void *ctx) : func(f), context(ctx) {}

    bool operator==(const MessageCallback &other) const
    {
        return func == other.func && context == other.context;
    }

    // 静态辅助方法：不分配内存，只返回一个值对象
    static MessageCallback Create(KernelMessageCallbackFunc f, void *ctx = nullptr)
    {
        return MessageCallback(f, ctx);
    }

    // 执行方法
    void invoke(const Message &msg) const
    {
        if (func)
        {
            // 在这里可以增加 Trace 调试，记录哪个 callback 被触发了
            func(msg, context);
        }
    }

    // 判断是否有效
    bool is_valid() const { return func != nullptr; }
};