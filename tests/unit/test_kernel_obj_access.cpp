#include "test_framework.hpp"
#include <kernel/MessageBus.hpp>
#include <kernel/ObjectFactory.hpp>

void test_message_bus_memory_stability()
{
    // 1. 模拟一个非 16 字节对齐的起始地址
    // 故意偏移 4 字节，看 ObjectFactory 是否能修正对齐
    uint8_t raw_buffer[2048];
    void *misaligned_base = raw_buffer + 4;

    PhysicalMemoryLayout layout{misaligned_base, 2000};
    ObjectFactory factory(layout);

    // 2. 连续创建对象
    auto *bus = factory.create<MessageBus>(&factory);

    // 3. 校验 MessageBus 的地址
    uintptr_t bus_addr = reinterpret_cast<uintptr_t>(bus);
    if (bus_addr % 16 != 0)
    {
        // 如果这里报错，说明 publish 里的 SSE 指令（由编译器优化生成）会崩
        throw std::runtime_error("MessageBus must be 16-byte aligned for safe CPU access");
    }

    // 4. 模拟写入动作
    Message msg;
    msg.type = static_cast<MessageType>(1);

    bus->publish(msg);
}

K_TEST_CASE("Integration: MessageBus Memory Alignment", test_message_bus_memory_stability);