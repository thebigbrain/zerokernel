#pragma once
#include "test_framework.hpp"
#include "kernel/Kernel.hpp"
#include "kernel/KernelProxy.hpp"
#include <kernel/StaticLayoutAllocator.hpp>
#include <kernel/MessageCallback.hpp>
#include <mock/MockKernel.hpp>

// 静态桥接函数

inline void unit_test_full_kernel_event_flow()
{
    // 1. 模拟物理内存环境
    uint8_t mem_buf[1024 * 1024];
    StaticLayoutAllocator loader(mem_buf, sizeof(mem_buf));

    // 2. 构造内核并初始化基础设施
    MockKernel kernel(nullptr, &loader);
    kernel.setup_infrastructure();

    // 3. 订阅事件 (利用 BIND_MESSAGE_CB)
    auto *bus = kernel.get_message_bus();
    bus->subscribe(MessageType::EVENT_PRINT, BIND_MESSAGE_CB(MockKernel, mock_print_bridge, &kernel));

    // 4. 模拟 RootTask 通过 Proxy 通信
    KernelProxy proxy(bus, nullptr);
    proxy.send_print_event("Full link integration test");

    // 5. 验证 (如果在控制台看到内核打印，则逻辑成立)
    // K_ASSERT( ... );
}