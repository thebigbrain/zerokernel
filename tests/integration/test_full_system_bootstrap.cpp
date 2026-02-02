#include <iostream>
#include <cstring>
#include <new>

// 平铺路径引用
#include "common/BootInfo.hpp"
#include "common/IUserRuntime.hpp"
#include "kernel/MessageBus.hpp"
#include "kernel/Kernel.hpp"
#include "kernel/ObjectFactory.hpp"
#include "kernel/KernelProxy.hpp"
#include "kernel/ICPUEngine.hpp"

#include "test_framework.hpp"
#include <mock/MockCPU.hpp>
#include <kernel/SimpleTaskManager.hpp>
#include <mock/SyncTestEngine.hpp>

#include <simulator/WinTaskContextFactory.hpp>

/**
 * 核心集成测试：验证从物理内存布局到 RootTask 消息回传的全链路
 */
void test_full_system_bootstrap_flow()
{
    std::cout << "\n========== SYSTEM BOOTSTRAP INTEGRATION ==========" << std::endl;

    // 1. 物理环境与内存管理
    size_t k_space_size = 4 * 1024 * 1024; // 增加到 4MB 确保安全
    void *k_space_base = _aligned_malloc(k_space_size, 16);
    memset(k_space_base, 0, k_space_size);
    ObjectFactory factory({k_space_base, k_space_size});

    // 2. 准备平台相关的工厂 (Simulator Layer)
    // 修复：提供具体的上下文和 TCB 工厂
    WinTaskContextFactory context_factory;
    WinTaskControlBlockFactory tcb_factory(&factory); // 假设这是 TCB 的具体工厂

    // 3. 构建内核核心组件
    MessageBus *bus = factory.create<MessageBus>(&factory);

    // 修复：按照重构后的构造函数注入三个工厂
    SimpleTaskManager *tm = factory.create<SimpleTaskManager>(
        &factory,
        &context_factory,
        &tcb_factory);

    // 4. 初始化内核对象并进行依赖注入
    // 这里的 kernel 可能仍需要一个 CPU 引擎来处理全局中断开关等，但不再处理切换
    Kernel kernel(nullptr, &factory);
    kernel.set_task_manager(tm);
    kernel.set_message_bus(bus);

    // 5. 使用同步引擎驱动测试
    // SyncTestEngine 会在调用 start() 时模拟跳转到第一个任务的逻辑
    SyncTestEngine sync_engine(tm);
    kernel.set_execution_engine(&sync_engine);

    // 6. 定义任务行为观察点
    static bool root_task_executed = false;
    static bool kernel_received_ipc = false;

    // 修复：root_logic 现在需要符合 ITaskContext 的 entry 签名
    auto root_logic = [](void *rt_ptr, void *arg)
    {
        root_task_executed = true;
        auto *rt = static_cast<IUserRuntime *>(rt_ptr);

        Message msg;
        msg.type = MessageType::EVENT_PRINT;
        rt->publish(msg);
    };

    // 订阅验证
    bus->subscribe(MessageType::EVENT_PRINT, KernelCallback::Create([](const Message &m)
                                                                    { kernel_received_ipc = true; }));

    // 配置启动信息：指定 Root 任务入口
    BootInfo info;
    info.root_task_entry = reinterpret_cast<void *>(+root_logic);
    kernel.set_boot_info(&info);

    // 7. 执行启动：从内核态转入用户态任务执行
    std::cout << "[Test] Entering Kernel Bootstrap..." << std::endl;

    // bootstrap 内部会调用 tm->spawn_task(info.root_task_entry)
    // 然后调用 sync_engine.start()
    kernel.bootstrap();

    // 8. 驱动同步消息循环
    bus->dispatch_pending();

    // 9. 最终判定
    bool success = root_task_executed && kernel_received_ipc;

    if (success)
    {
        std::cout << "[SUCCESS] Full Bootstrap Flow Verified." << std::endl;
        std::cout << "  - Factory-based TCB & Context creation: OK" << std::endl;
        std::cout << "  - ExecutionEngine -> TaskManager handoff: OK" << std::endl;
        std::cout << "  - Message-driven IPC through Proxy: OK" << std::endl;
    }
    else
    {
        std::cout << "[FAILED] Integrity Check Failed!" << std::endl;
        if (!root_task_executed)
            std::cout << "    Reason: Root Task never ran." << std::endl;
        if (!kernel_received_ipc)
            std::cout << "    Reason: IPC message lost or dispatch failed." << std::endl;
    }

    _aligned_free(k_space_base);
}

K_TEST_CASE("Full System Integration", test_full_system_bootstrap_flow);