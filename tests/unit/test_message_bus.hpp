// unit/test_message_bus.hpp
#pragma once

#include "test_framework.hpp"

#include "common/Message.hpp"
#include <kernel/StaticLayoutAllocator.hpp>
#include <kernel/KernelObjectBuilder.hpp>
#include <kernel/MessageBus.hpp>

#include "unit/test_message_callback.hpp" // 复用 mock_kernel_handler

inline void unit_test_message_bus_logic()
{
    uint8_t buf[1024];
    StaticLayoutAllocator loader(buf, 1024);
    KernelObjectBuilder builder(&loader);

    // 构造 Bus (假设 MessageBus 接受 builder 以便管理内部 KList)
    auto *bus = builder.construct<MessageBus>(&builder);

    g_callback_executed = false;

    // 1. 订阅：传入值对象
    MessageCallback cb(mock_kernel_handler, nullptr);
    bus->subscribe(MessageType::EVENT_PRINT, cb);

    // 2. 发布
    Message msg;
    msg.type = MessageType::EVENT_PRINT;
    bus->publish(msg);

    K_ASSERT(g_callback_executed, "MessageBus failed to route message to Callback struct!");
}