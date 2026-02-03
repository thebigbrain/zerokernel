#pragma once

#include <new>

#include "common/BootInfo.hpp"
#include "TaskService.hpp"
#include "IExecutionEngine.hpp"
#include "IMessageBus.hpp"
#include "ICPUEngine.hpp"
#include "IObjectBuilder.hpp" // 引入 Builder 接口
#include "ITaskControlBlockFactory.hpp"
#include "ITaskContextFactory.hpp"
#include "KStackBuffer.hpp"

class Kernel
{
private:
    // 基础依赖
    ICPUEngine *_cpu;
    IAllocator *_static_allocator; // 初始静态分配器
    IAllocator *_runtime_heap;     // 稍后建立的动态堆
    IObjectBuilder *_builder;      // 稍后建立的业务构建器

    ITaskContextFactory *_task_context_factory;
    ITaskControlBlockFactory *_tcb_factory;

    // 领域组件
    TaskService *_task_service;
    IExecutionEngine *_engine;
    IMessageBus *_bus;
    ITaskLifecycle *_lifecycle;
    ISchedulingStrategy *_strategy;

    BootInfo *_boot_info = nullptr;
    IUserRuntime *_user_runtime = nullptr;

public:
    // 构造函数：注入 Builder 和 CPU 引擎
    Kernel(ICPUEngine *cpu, IAllocator *loader)
        : _cpu(cpu),
          _static_allocator(loader) {}

    void set_boot_info(BootInfo *info) { _boot_info = info; }
    void set_context_factory(ITaskContextFactory *factory) { _task_context_factory = factory; }

    // 核心初始化逻辑
    void setup_infrastructure();
    void spawn_initial_tasks();
    void start_engine();

    void bootstrap();

    IMessageBus *get_message_bus() const { return _bus; }
    ITaskLifecycle *get_task_lifecycle() const { return _lifecycle; }

private:
    void handle_event_print(const Message &msg);

    KStackBuffer *create_stack(size_t size)
    {
        // 将“去哪里拿内存”和“怎么构建对象”的细节锁死在这里
        return _builder->construct<KStackBuffer>(_runtime_heap, size);
    }
};