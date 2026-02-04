// unit/test_kernel_proxy.hpp
#pragma once

#include "test_framework.hpp"
#include "kernel/KernelProxy.hpp"
#include "common/Message.hpp"

// 1. 简单的 Mock 实现，用于捕获 Proxy 的输出
class MockMessageBus : public IMessageBus
{
public:
    MessageType last_published_type = MessageType::NONE;
    bool publish_called = false;

    void publish(const Message &msg) override
    {
        publish_called = true;
        last_published_type = msg.type;
        // 如果需要，可以在这里校验 msg.data 里的字符串内容
    }

    // 单元测试中暂时不需要实现 subscribe/unsubscribe
    void subscribe(MessageType type, MessageCallback cb) override {}
    void unsubscribe(MessageType type, MessageCallback cb) override {}
};

class MockSchedulingControl : public ISchedulingControl
{
public:
    bool yield_called = false;
    bool terminate_called = false;

    void yield_current_task() override
    {
        yield_called = true;
    }

    void terminate_current_task() override
    {
        terminate_called = true;
    }
};

inline void unit_test_kernel_proxy_behavior()
{
    // 准备 Mock 环境
    MockMessageBus mock_bus;
    MockSchedulingControl mock_sched;

    // 注入 Mock 依赖
    KernelProxy proxy(&mock_bus, &mock_sched);

    // --- 测试场景 1: 消息发布 ---
    // 构造一个业务消息
    Message msg;
    msg.type = MessageType::EVENT_PRINT;
    // 假设此处模拟 RootTask 发送 print
    proxy.publish(msg);

    K_ASSERT(mock_bus.publish_called, "Proxy should call bus->publish");
    K_ASSERT(mock_bus.last_published_type == MessageType::EVENT_PRINT, "Proxy failed to pass correct MessageType");

    // --- 测试场景 2: 协作调度 ---
    proxy.yield();
    K_ASSERT(mock_sched.yield_called, "Proxy should forward yield call to ISchedulingControl");
}