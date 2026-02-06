// unit/test_kernel_proxy.hpp
#pragma once

#include "test_framework.hpp"
#include "mock/mock.hpp"
#include "kernel/KernelProxy.hpp"
#include "common/Message.hpp"

#include "mock/MockSchedulingControl.hpp"

void unit_test_kernel_proxy_behavior()
{
    Mock mock;
    auto *kernel = mock.kernel();
    kernel->setup_infrastructure();

    KernelInspector ki(kernel);

    // 准备 Mock 环境
    MockSchedulingControl mock_sched;
    auto *bus = ki.bus();

    // 注入 Mock 依赖
    KernelProxy proxy(bus, &mock_sched);

    // --- 测试场景 1: 消息发布 ---
    // 构造一个业务消息
    Message msg;
    msg.type = MessageType::EVENT_PRINT;
    // 假设此处模拟 RootTask 发送 print
    proxy.publish(msg);

    // --- 测试场景 2: 协作调度 ---
    proxy.yield();
    K_T_ASSERT(mock_sched.yield_called, "Proxy should forward yield call to ISchedulingControl");
}