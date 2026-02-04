// unit/test_message_callback.hpp
#pragma once

#include "test_framework.hpp"

#include "kernel/MessageCallback.hpp"

// 静态测试函数，用于修改外部状态
static bool g_callback_executed = false;
inline void mock_kernel_handler(const Message &msg, void *ctx)
{
    g_callback_executed = true;
}

inline void unit_test_message_callback_invoke()
{
    g_callback_executed = false;

    // 1. 创建回调对象 (值传递)
    MessageCallback cb = MessageCallback::Create(mock_kernel_handler, nullptr);

    // 2. 模拟消息
    Message msg;
    msg.type = MessageType::EVENT_PRINT; // 假设你有这个枚举

    // 3. 执行
    cb.invoke(msg);

    K_ASSERT(g_callback_executed, "MessageCallback failed to invoke function pointer!");
}