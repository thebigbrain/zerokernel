#pragma once
#include "test_framework.hpp"
#include "kernel/ITaskContext.hpp"
#include "kernel/IIdGenerator.hpp"
#include "kernel/ITaskContextFactory.hpp"
#include "kernel/SimpleTaskFactory.hpp"
#include <kernel/StaticLayoutAllocator.hpp>
#include <kernel/KernelObjectBuilder.hpp>
#include <kernel/KStackBuffer.hpp>
#include <kernel/BitmapIdGenerator.hpp>

#include <mock/MockTaskContext.hpp>
#include <mock/MockTaskContextFactory.hpp>

inline void unit_test_task_factory_integrity()
{
    // 1. 准备基础设施
    uint8_t scratch[4096];
    StaticLayoutAllocator loader(scratch, 4096);
    KernelObjectBuilder builder(&loader);

    // 2. 准备依赖组件
    auto *ctx_factory = builder.construct<MockTaskContextFactory>();
    auto *id_gen = builder.construct<BitmapIdGenerator<64>>();

    // 3. 构造工厂 (核心：检查这里是否正确传入了所有指针)
    // 假设 SimpleTaskFactory 的构造函数签名是：
    // SimpleTaskFactory(IObjectBuilder* b, ITaskContextFactory* cf, ITaskIdGenerator* ig, void* er)
    auto *factory = builder.construct<SimpleTaskFactory>(
        &builder,
        ctx_factory,
        id_gen);

    // 4. 模拟任务配置
    TaskExecutionInfo exec{(TaskEntry)0x1234, nullptr, nullptr};

    // 准备模拟栈空间 (使用 KStackBuffer)
    auto *stack = builder.construct<KStackBuffer>(&loader, 4096);
    TaskResourceConfig res{TaskPriority::NORMAL, stack};

    // 5. 执行创建并断言
    std::cout << "[Step] Testing TCB creation..." << std::endl;
    ITaskControlBlock *tcb = factory->create_tcb(exec, res);

    K_T_ASSERT(tcb != nullptr, "Factory failed to create TCB");

    std::cout << "[PASS] SimpleTaskFactory integrity verified." << std::endl;
}