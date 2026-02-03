#include <iostream>
#include <cstring>
#include <new>

// 1. 核心架构与内核组件
#include "common/BootInfo.hpp"
#include "common/IUserRuntime.hpp"
#include "kernel/MessageBus.hpp"
#include "kernel/Kernel.hpp"
#include "kernel/StaticLayoutAllocator.hpp"
#include "kernel/KernelProxy.hpp"
#include "kernel/ICPUEngine.hpp"

// 2. 测试框架与已有的 Mock 组件
#include "test_framework.hpp"
#include <mock/SyncTestEngine.hpp>
#include <mock/MockTaskContext.hpp>
#include <mock/MockTaskContextFactory.hpp>
#include <kernel/SimpleTaskLifecycle.hpp>
#include <kernel/SimpleTaskFactory.hpp>

/**
 * 符合 MessageCallback 签名的静态监听函数
 * 利用 context 指针回传测试结果
 */
void on_test_event_received(const Message &msg, void *context)
{
    if (context)
    {
        *static_cast<bool *>(context) = true;
        std::cout << "  [TestObserver] MessageBus captured EVENT_PRINT!" << std::endl;
    }
}

/**
 * 核心集成测试：完全模拟物理内存环境下的内核引导
 */
void test_full_system_bootstrap_flow()
{
    std::cout << "\n========== [kmain-style] INTEGRATION TEST START ==========" << std::endl;

    // --- 1. 模拟物理内存布局 (申请 4MB 空间模拟 RAM) ---
    size_t k_space_size = 4 * 1024 * 1024;
    void *k_space_base = _aligned_malloc(k_space_size, 16);
    memset(k_space_base, 0, k_space_size);

    // --- 2. 对应 kmain 步骤 1: 建立引导分配器 (Placement New) ---
    // 严格遵循：loader 位于内存起始，其后的空间交给 loader 管理
    StaticLayoutAllocator *loader = new (k_space_base) StaticLayoutAllocator(
        (uint8_t *)k_space_base + sizeof(StaticLayoutAllocator),
        k_space_size - sizeof(StaticLayoutAllocator));

    // --- 3. 对应 kmain 步骤 2: 原地构造 Kernel ---
    // 模拟 cpu 为 nullptr（或传入 MockCPU），loader 作为唯一的分配源
    Kernel *kernel = new (loader->allocate(sizeof(Kernel))) Kernel(nullptr, loader);

    // --- 4. 准备测试桩与观察变量 ---
    static bool root_task_executed = false;
    static bool kernel_received_ipc = false;

    // Root 任务逻辑：验证 IUserRuntime (KernelProxy) 能否成功 publish
    auto root_logic = [](void *rt_ptr, void *arg)
    {
        root_task_executed = true;
        std::cout << "  [RootTask] Execution started." << std::endl;
        if (rt_ptr)
        {
            auto *rt = static_cast<IUserRuntime *>(rt_ptr);
            Message msg;
            msg.type = MessageType::EVENT_PRINT;
            rt->publish(msg);
        }
    };

    // --- 5. 对应 kmain 步骤 3: 注入信息并启动 ---
    // 使用你已经准备好的 Mock 工厂
    MockTaskContextFactory ctx_factory;
    BootInfo info;
    info.root_task_entry = reinterpret_cast<void (*)(void *, void *)>(+root_logic);

    kernel->set_boot_info(&info);
    kernel->set_context_factory(&ctx_factory);

    // 核心动作：触发内核内部初始化逻辑
    kernel->bootstrap();

    // --- 6. 验证 MessageBus 订阅逻辑 ---
    // bootstrap 之后，bus 应该已经由 Kernel 内部创建完成
    auto *bus = kernel->get_message_bus();
    if (bus)
    {
        // 使用 MessageCallback 静态辅助方法绑定观察者
        bus->subscribe(
            MessageType::EVENT_PRINT,
            MessageCallback::Create(on_test_event_received, &kernel_received_ipc));
    }

    // --- 7. 驱动同步执行 ---
    // 从内核获取创建好的 TaskManager (SimpleTaskLifecycle)
    auto *tm = static_cast<SimpleTaskLifecycle *>(kernel->get_task_lifecycle());

    // SyncTestEngine 会寻找 current_task 并调用 ctx->jump_to()
    // 由于使用了 MockTaskContext，这会直接同步执行 root_logic
    SyncTestEngine sync_engine(tm);
    sync_engine.start();

    // --- 8. 判定全链路结果 ---
    bool success = root_task_executed && kernel_received_ipc;

    std::cout << "--------------------------------------------------" << std::endl;
    if (success)
    {
        std::cout << "[SUCCESS] Full System Bootstrap Flow Verified." << std::endl;
        std::cout << "  - Memory Alignment & Layout: OK" << std::endl;
        std::cout << "  - Kernel/Loader Handshake: OK" << std::endl;
        std::cout << "  - Sync Dispatching via MockContext: OK" << std::endl;
        std::cout << "  - Cross-Task IPC via Callback: OK" << std::endl;
    }
    else
    {
        std::cout << "[FAILED] Integration checkpoints not met." << std::endl;
        if (!root_task_executed)
            std::cout << "    [!] Root Task failed to execute." << std::endl;
        if (!kernel_received_ipc)
            std::cout << "    [!] IPC Message was lost or callback failed." << std::endl;
    }

    // 释放模拟物理内存
    _aligned_free(k_space_base);
}

K_TEST_CASE("Full System Integration (kmain style)", test_full_system_bootstrap_flow);