#pragma once

#include <iostream>
#include <vector>
#include "common/IUserRuntime.hpp"

// 模拟内核提供的 Runtime 实现
class MockUserRuntime : public IUserRuntime
{
public:
    std::vector<Message> sent_messages;
    bool yield_called = false;

    virtual void publish(const Message &msg) override
    {
        // 捕获 RootTask 发出的消息
        sent_messages.push_back(msg);

        // 简单演示：如果是打印消息，直接输出
        if (msg.type == MessageType::EVENT_PRINT)
        {
            char buf[9] = {0};
            memcpy(buf, &msg.payload[0], 8);
            std::cout << "[Mock Kernel] Received Print: " << buf << std::endl;
        }
    }

    virtual void yield() override
    {
        yield_called = true;
    }
};

// 模拟寄存器上下文结构 (符合 WinX64 调用约定)
struct MockRegisters
{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax; // RCX 是第一个参数
    uint64_t rip;
};