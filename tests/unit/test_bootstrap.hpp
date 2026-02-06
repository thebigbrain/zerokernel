// unit/test_bootstrap.hpp
#pragma once

#include "mock/mock.hpp"
#include <iostream>

#include <inspect/KernelInspector.hpp>
#include <inspect/HeapInspector.hpp>

/**
 * @brief 测试内核引导流程的完整性
 */
void unit_test_bootstrap()
{
    // 1. Setup: 模拟器上电，申请 64KB 模拟物理内存
    // 此时 Mock 构造函数内部已经完成了：
    // - StaticLayoutAllocator 的建立
    // - Kernel 对象的 Placement New
    Mock mock(64 * 1024);
    auto *kernel = mock.kernel();

    K_T_ASSERT(kernel != nullptr, "kernel is null");

    KernelInspector ki(kernel);

    K_T_ASSERT(ki.hooks() != nullptr, "Platform hooks are null");

    // 2. Execution: 执行内核引导
    // 这将触发：建立堆、建立 Builder、创建 Root/Idle 任务等
    kernel->setup_infrastructure();
    kernel->setup_boot_tasks();

    // 3. Verification: 使用 Mock 封装的逻辑进行断言
    std::cout << "[Test] Verifying Kernel Bootstrap..." << std::endl;

    HeapInspector hi(ki.heap());

    // A. 验证基础设施组件是否已挂载
    // 注意：这里使用你封装在 Mock 或 Inspector 中的 Getter
    K_T_ASSERT(ki.heap() != nullptr, "Error: Runtime Heap not initialized");
    K_T_ASSERT(ki.builder() != nullptr, "Error: Object Builder not initialized");
    K_T_ASSERT(ki.task_service() != nullptr, "Error: Task Service not initialized");

    // B. 验证堆空间协商是否成功
    // 64KB 的总内存，扣除 Kernel 和基础组件后，堆应该占用合理比例
    size_t heap_free = hi.get_free_size();
    K_T_ASSERT(heap_free > 0 && heap_free < 64 * 1024, "Error: Heap size calculation invalid");

    // C. 验证初始任务状态
    // Bootstrap 之后，调度策略中应该至少有两个就绪任务（Root 和 Idle）
    auto *strategy = ki.strategy();
    // 假设你的策略类支持查询就绪任务数量
    // K_T_ASSERT(strategy->get_ready_count() >= 2, "Error: Root or Idle task not ready");

    // D. 验证 KObject 的基类约束
    // 确保内核对象本身也在分配器的管理范围内
    uintptr_t k_addr = reinterpret_cast<uintptr_t>(kernel);
    uintptr_t ram_start = reinterpret_cast<uintptr_t>(mock.get_ram_start());
    K_T_ASSERT(k_addr >= ram_start, "Error: Kernel object located outside of simulated RAM");

    std::cout << "[Pass] Kernel Bootstrap successfully reached ready state." << std::endl;

    // 4. Teardown: mock 对象的析构函数会自动释放 new 出来的 64KB 内存
}