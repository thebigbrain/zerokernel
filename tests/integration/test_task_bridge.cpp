// tests/integration/test_task_bridge.cpp
#include "test_framework.hpp"
#include <simulator/WinCPUEngine.hpp>
#include <kernel/MessageBus.hpp>
#include <simulator/WinTaskContext.hpp>

void test_shadow_space_presence()
{
    // 直接使用具体的实现类进行测试
    WinTaskContext ctx;

    // Windows x64 必须 16 字节对齐
    alignas(16) uint8_t stack[1024];
    void *stack_top = stack + 1024;

    // 修复：模拟设置流。注意：WinTaskContext 内部现在应该负责 Shadow Space
    ctx.setup_flow((void (*)())0x123, stack_top, (void (*)())0x456);

    // 获取 RSP
    uintptr_t sp = (uintptr_t)ctx.get_stack_pointer();

    // Windows x64 ABI 核心：
    // [High] stack_top
    // [....] Return Address (8 bytes)
    // [Low ] Shadow Space (32 bytes) <-- RSP 应该在这里或之下
    uintptr_t stack_limit = (uintptr_t)stack_top;

    // 计算从 RSP 到 stack_top 的总空间
    // 扣除掉返回地址占用的 8 字节，剩下的必须能够容纳 32 字节 Shadow Space
    if (stack_limit - (sp + 8) < 32)
    {
        throw std::runtime_error("Shadow space (32 bytes) is missing! Current offset: " + std::to_string(stack_limit - sp));
    }
}

// 修复 2: 强制 16 字节对齐检测
void test_object_alignment_strict()
{
    uint8_t *buf = (uint8_t *)malloc(2048);
    // 确保基地址是对齐的，方便测试分配器算法
    uintptr_t aligned_base = ((uintptr_t)buf + 15) & ~15;
    ObjectFactory factory({(void *)aligned_base, 1024});

    for (int i = 0; i < 10; ++i)
    {
        // 申请不同大小的空间，验证分配器是否强制补齐到 16 字节
        void *p = factory.allocate_raw(i + 1);
        if ((uintptr_t)p % 16 != 0)
        {
            free(buf);
            throw std::runtime_error("Alignment violated for size " + std::to_string(i + 1));
        }
    }
    free(buf);
}

// 修复 3: 模拟 Proxy 调用的完整性
void test_task_proxy_call_integrity()
{
    // 使用 malloc 模拟物理内存
    void *fake_phys = malloc(1024 * 1024);
    ObjectFactory factory({fake_phys, 1024 * 1024});

    // 创建 MessageBus
    MessageBus *bus = factory.create<MessageBus>(&factory);

    // 模拟 Task 的 RCX 传参
    void *task_arg_proxy = (void *)bus;

    try
    {
        Message msg;
        msg.type = static_cast<MessageType>(0x55);

        // 核心检查：通过 proxy 指针调用方法
        MessageBus *proxy_bus = static_cast<MessageBus *>(task_arg_proxy);
        proxy_bus->publish(msg);

        if (proxy_bus->get_pending_count() != 1)
        {
            throw std::runtime_error("Message count mismatch");
        }
    }
    catch (...)
    {
        free(fake_phys);
        throw;
    }

    free(fake_phys);
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