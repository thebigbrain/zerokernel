// unit/test_kernel_proxy.hpp
#pragma once

#include "test_framework.hpp"
#include "kernel/KernelProxy.hpp"
#include "common/Message.hpp"

#include "mock/MockMessageBus.hpp"
#include "mock/MockSchedulingControl.hpp"

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