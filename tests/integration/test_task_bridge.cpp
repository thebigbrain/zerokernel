// tests/integration/test_task_bridge.cpp
#include "test.hpp"
#include <simulator/WinCPUEngine.hpp>
#include <kernel/MessageBus.hpp>

void test_task_proxy_call_integrity()
{
    // 1. 初始化环境
    uint8_t *fake_phys = (uint8_t *)malloc(1024 * 1024);
    PhysicalMemoryLayout layout{fake_phys, 1024 * 1024};
    ObjectFactory factory(layout);

    // 2. 创建内核组件
    MessageBus *bus = factory.create<MessageBus>(&factory);

    // 3. 模拟 Proxy 传递给任务的过程
    // 这里的 bus 指针就是将来要通过 RCX 传给 root_task 的值
    void *task_arg_proxy = (void *)bus;

    // 4. 模拟任务内部的动作
    // 之前你崩在 publish，这里我们模拟任务内部调用 publish
    try
    {
        Message msg;
        msg.type = static_cast<MessageType>(0x55);

        // 验证这个指针在被当做 proxy 传递后是否依然能正常操作其内部成员
        MessageBus *proxy_bus = static_cast<MessageBus *>(task_arg_proxy);
        proxy_bus->publish(msg);

        if (proxy_bus->get_pending_count() != 1)
        {
            throw std::runtime_error("Message was lost through proxy call");
        }
    }
    catch (...)
    {
        throw std::runtime_error("Crash detected during proxy-based publish!");
    }

    free(fake_phys);
}

void test_shadow_space_presence()
{
    WinCPUEngine cpu;
    void *ctx_mem = malloc(cpu.get_context_size());
    ITaskContext *ctx = cpu.create_context_at(ctx_mem);

    uint8_t stack[1024];
    void *stack_top = stack + 1024;

    ctx->prepare((void (*)())0x123, stack_top, (void (*)())0x456);

    // 获取当前模拟的 RSP
    uintptr_t sp = (uintptr_t)ctx->get_stack_pointer();

    // 在 Windows x64 ABI 中，RSP 指向返回地址。
    // 返回地址之上（高地址）必须至少有 32 字节的空间。
    uintptr_t shadow_bottom = sp + 8;
    uintptr_t stack_limit = (uintptr_t)stack_top;

    if (stack_limit - shadow_bottom < 32)
    {
        throw std::runtime_error("Shadow space (32 bytes) is missing in stack frame!");
    }

    free(ctx_mem);
}

void test_object_alignment_strict()
{
    uint8_t buf[1024];
    ObjectFactory factory({buf, 1024});

    for (int i = 0; i < 10; ++i)
    {
        void *p = factory.allocate_raw(sizeof(uint64_t)); // 申请小空间
        if ((uintptr_t)p % 16 != 0)
        {
            throw std::runtime_error("Strict 16-byte alignment violated!");
        }
    }
}

void test_task_proxy_bridge_enhanced()
{
    uint8_t *fake_phys = (uint8_t *)malloc(1024 * 1024);
    ObjectFactory factory({fake_phys, 1024 * 1024});

    std::cout << "  [CHECK] Creating Bus..." << std::endl;
    auto *bus = factory.create<MessageBus>(&factory);

    // 关键诊断：不直接调用 publish，而是测试它的依赖项
    std::cout << "  [CHECK] Testing KList push_back..." << std::endl;

    // 假设 MessageBus 内部使用 KList<Message> 或类似物
    // 我们在这里手动测试一次分配和插入
    struct MockNode
    {
        Message m;
        MockNode *next;
    };
    void *node_mem = factory.allocate_raw(sizeof(MockNode));
    if (!node_mem)
        throw std::runtime_error("Factory OOM in test");

    std::cout << "  [CHECK] Calling bus->publish()..." << std::endl;
    Message msg;
    msg.type = static_cast<MessageType>(0xAA);

    // 如果这里卡住，说明是 publish 内部的循环或锁问题
    bus->publish(msg);

    std::cout << "  [CHECK] Done!" << std::endl;
    free(fake_phys);
}

void test_task_proxy_bridge_final()
{
    uint8_t *fake_phys = (uint8_t *)malloc(1024 * 1024);
    ObjectFactory factory({fake_phys, 1024 * 1024});

    auto *bus = factory.create<MessageBus>(&factory);

    // 探测点：检查关键成员是否初始化
    // 假设 MessageBus 暴露了查看队列是否为空的接口
    // 如果没有，可以尝试打印 bus 的地址空间
    std::cout << "  [DEBUG] Bus Address: " << bus << std::endl;

    Message msg;
    msg.type = static_cast<MessageType>(0xAA);

    // 尝试执行，并捕获可能的硬件异常
    // 在模拟器环境下，如果发生 Access Violation，通常会跳出 debugger
    bus->publish(msg);

    std::cout << "  [DEBUG] Publish returned successfully." << std::endl;

    free(fake_phys);
}

K_TEST_CASE("Integration: Task Proxy Bridge Final", test_task_proxy_bridge_final);
K_TEST_CASE("Integration: Task Proxy Bridge Enhanced", test_task_proxy_bridge_enhanced);
K_TEST_CASE("ObjectFactory: Strict 16-Byte Alignment", test_object_alignment_strict);
K_TEST_CASE("Simulator: ABI Shadow Space Check", test_shadow_space_presence);
K_TEST_CASE("Integration: Task Proxy Bridge", test_task_proxy_call_integrity);