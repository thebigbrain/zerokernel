#pragma once
#include "test_framework.hpp"
#include "kernel/Kernel.hpp"

class MockKernel : public Kernel
{
public:
    using Kernel::Kernel; // 继承构造函数

    bool print_called = false;
    char last_message[256] = {0};

    // 暴露一个公共的桥接方法
    void mock_print_bridge(const Message &msg, void *ctx)
    {
        auto *self = static_cast<MockKernel *>(ctx);
        self->print_called = true;
        // 假设 msg.data 包含字符串指针
        // strncpy(self->last_message, (const char*)msg.data, 255);

        // 仍然可以调用基类的私有方法来验证逻辑
        self->handle_event_print(msg);
    }
};