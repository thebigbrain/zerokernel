// unit/test_message_system.hpp
#pragma once
#include "test_framework.hpp"
#include "kernel/MessageBus.hpp" // 确保可以访问到 Bus 和相关的类型定义
#include <kernel/StaticLayoutAllocator.hpp>
#include <kernel/KernelObjectBuilder.hpp>

class DomainServiceMock
{
public:
    int call_count = 0;
    MessageType last_type;

    void on_dispatch(const Message &msg)
    {
        call_count++;
        last_type = msg.type;
    }
};

inline void unit_test_message_system_integrity()
{
    // 1. 环境准备
    uint8_t scratch[2048];
    StaticLayoutAllocator loader(scratch, 2048);
    KernelObjectBuilder builder(&loader);

    // 2. 构造领域模型
    auto *bus = builder.construct<MessageBus>(&builder);
    DomainServiceMock service;

    // 3. 验证宏绑定与订阅
    // 这里模拟的就是 Kernel::setup_infrastructure 里的真实逻辑
    bus->subscribe(
        MessageType::EVENT_PRINT,
        BIND_MESSAGE_CB(DomainServiceMock, on_dispatch, &service));

    // 4. 模拟事件触发
    Message test_msg;
    test_msg.type = MessageType::EVENT_PRINT;

    bus->publish(test_msg);
    bus->dispatch_messages();

    K_T_ASSERT(service.call_count == 1, "Even manual invoke failed! Macro is broken.");
    service.call_count = 0; // 重置，继续测 Bus

    MessageCallback manual_cb = BIND_MESSAGE_CB(DomainServiceMock, on_dispatch, &service);
    manual_cb.invoke(test_msg);
    // 5. 领域规则断言
    K_T_ASSERT(service.call_count == 1, "MessageBus failed to deliver event through BIND macro");
    K_T_ASSERT(service.last_type == MessageType::EVENT_PRINT, "Message content corruption during dispatch");
}