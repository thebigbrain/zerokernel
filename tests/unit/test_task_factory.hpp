#pragma once
#include "test_framework.hpp"
#include "kernel/ITaskContext.hpp"
#include "kernel/IIdGenerator.hpp"
#include "kernel/ITaskContextFactory.hpp"
#include "kernel/SimpleTaskFactory.hpp"
#include <kernel/StaticLayoutAllocator.hpp>
#include <kernel/KernelObjectBuilder.hpp>
#include <kernel/KStackBuffer.hpp>

// 1. Mock ID 生成器
class MockIdGen : public IIdGenerator
{
public:
    uint32_t acquire() override { return 42; }
    void release(uint32_t id) override {}

    bool is_active(uint32_t id) const
    {
        return true;
    }
};

// 2. Mock 上下文
class MockContext : public ITaskContext
{
public:
    void setup_flow(void (*entry)(), void *stack_top, void (*exit)()) override {}
    void load_argument(size_t index, uintptr_t value) override {}
    void *get_stack_pointer() const override { return nullptr; }

    size_t get_context_size() const { return 0; }
    void transit_to(ITaskContext *target) {}
    void jump_to() {}
};

// 3. Mock 上下文工厂
class MockContextFactory : public ITaskContextFactory
{
public:
    ITaskContext *create_context() override
    {
        // 注意：这里可能需要从 builder 构造以符合你的内存追踪
        return new MockContext();
    }
    void destroy_context(ITaskContext *ctx) override { delete ctx; }
};

inline void unit_test_task_factory_integrity()
{
    // 1. 准备基础设施
    uint8_t scratch[4096];
    StaticLayoutAllocator loader(scratch, 4096);
    KernelObjectBuilder builder(&loader);

    // 2. 准备依赖组件
    auto *ctx_factory = builder.construct<MockContextFactory>();
    auto *id_gen = builder.construct<MockIdGen>();
    void *exit_router = (void *)(uintptr_t)0xDEADBEEF;

    // 3. 构造工厂 (核心：检查这里是否正确传入了所有指针)
    // 假设 SimpleTaskFactory 的构造函数签名是：
    // SimpleTaskFactory(IObjectBuilder* b, ITaskContextFactory* cf, ITaskIdGenerator* ig, void* er)
    auto *factory = builder.construct<SimpleTaskFactory>(
        &builder,
        ctx_factory,
        id_gen,
        exit_router);

    // 4. 模拟任务配置
    TaskExecutionInfo exec{(TaskEntry)0x1234, nullptr, nullptr};

    // 准备模拟栈空间 (使用 KStackBuffer)
    auto *stack = builder.construct<KStackBuffer>(&loader, 4096);
    TaskResourceConfig res{TaskPriority::NORMAL, stack};

    // 5. 执行创建并断言
    std::cout << "[Step] Testing TCB creation..." << std::endl;
    ITaskControlBlock *tcb = factory->create_tcb(exec, res);

    K_ASSERT(tcb != nullptr, "Factory failed to create TCB");
    K_ASSERT(tcb->get_id() == 42, "ID Generator not properly utilized");

    std::cout << "[PASS] SimpleTaskFactory integrity verified." << std::endl;
}