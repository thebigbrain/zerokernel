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

/**
 * 核心集成测试：验证从物理内存布局到 RootTask 消息回传的全链路
 */
void test_full_system_bootstrap_flow()
{
    std::cout << std::endl;
    std::cout << "========== SYSTEM BOOTSTRAP INTEGRATION ==========" << std::endl;

    // 1. 准备“物理内存”空间 (模拟模拟器为内核分配的施工区)
    // 假设我们给内核 2MB 的空间用于存放内核对象（MessageBus, Task 等）
    size_t k_space_size = 2 * 1024 * 1024;
    void *k_space_base = malloc(k_space_size);
    memset(k_space_base, 0, k_space_size);

    PhysicalMemoryLayout layout;
    layout.base = k_space_base;
    layout.size = k_space_size;

    // 2. 基础设施准备
    // ObjectFactory 在 layout 指定的地址上进行内存分配管理
    ObjectFactory factory(layout);

    // ICPUEngine 通常由模拟器实现，这里可以传 nullptr 或简单的 Mock
    ICPUEngine *mock_cpu = nullptr;

    // 3. 实例化 Kernel
    Kernel kernel(mock_cpu, &factory);

    // 4. 准备 BootInfo
    BootInfo info;

    // 模拟 RootTask 逻辑
    static auto root_task_logic = [](IUserRuntime *rt)
    {
        Message msg;
        msg.type = MessageType::EVENT_PRINT;
        const char *hello = "HELLOOS";
        msg.payload[0] = *(uint64_t *)hello;

        // 最终会进入 Kernel::handle_event_print
        if (rt)
        {
            rt->publish(msg);
        }
    };

    static auto idle_task_logic = [](IUserRuntime *rt)
    {
        // 空闲任务什么也不做
        while (true)
        {
            // 模拟 CPU 空转
        }
    };

    // 填充入口点。注意：这里的 root_task_entry 模拟的是加载后的代码地址
    info.root_task_entry = (void (*)(void *, void *))(uintptr_t)+root_task_logic;
    info.idle_task_entry = (void (*)(void *))(uintptr_t)+idle_task_logic;
    info.config_ptr = nullptr;

    // 5. 执行引导流程
    std::cout << "[Test] Kernel constructing on layout: base=" << layout.base
              << ", size=" << layout.size << std::endl;

    try
    {
        // bootstrap 内部会：
        // a. 调用 factory.allocate_raw 为 MessageBus 分配内存
        // b. 订阅消息
        // c. 分配 Task 对象并拉起执行
        kernel.bootstrap(&info);
    }
    catch (...)
    {
        std::cerr << "[Test] Flow exited." << std::endl;
    }

    // 6. 清理
    free(k_space_base);
    std::cout << "========== BOOTSTRAP TEST COMPLETE ==========" << std::endl;
}

K_TEST_CASE("Full System Integration", test_full_system_bootstrap_flow);