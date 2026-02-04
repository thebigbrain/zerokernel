#pragma once

#include <new>

#include "common/BootInfo.hpp"
#include "StaticLayoutAllocator.hpp"
#include "TaskService.hpp"
#include "IExecutionEngine.hpp"
#include "IMessageBus.hpp"
#include "ICPUEngine.hpp"
#include "IObjectBuilder.hpp" // 引入 Builder 接口
#include "ITaskControlBlockFactory.hpp"
#include "ITaskContextFactory.hpp"
#include "KStackBuffer.hpp"
#include "ISchedulingControl.hpp"
#include "IExecutionEngine.hpp"
#include "KernelHeapAllocator.hpp"

class Kernel
{
private:
    // 基础依赖
    ICPUEngine *_cpu;
    StaticLayoutAllocator *_static_allocator; // 初始静态分配器
    IAllocator *_runtime_heap;                // 稍后建立的动态堆
    IObjectBuilder *_builder;                 // 稍后建立的业务构建器

    ITaskContextFactory *_task_context_factory;
    ITaskControlBlockFactory *_tcb_factory;

    // 领域组件
    TaskService *_task_service;
    IExecutionEngine *_engine;
    IMessageBus *_bus;
    ITaskLifecycle *_lifecycle;
    ISchedulingStrategy *_strategy;
    ISchedulingControl *_scheduling_control;

    BootInfo *_boot_info = nullptr;
    IUserRuntime *_user_runtime = nullptr;

public:
    // 构造函数：注入 Builder 和 CPU 引擎
    Kernel(StaticLayoutAllocator *static_allocator, ICPUEngine *cpu = nullptr)
        : _cpu(cpu),
          _static_allocator(static_allocator),
          _runtime_heap(nullptr), // 必须手动清零
          _builder(nullptr),
          _task_service(nullptr),
          _engine(nullptr), // 关键！
          _bus(nullptr),
          _lifecycle(nullptr),
          _strategy(nullptr)
    {
    }

    void set_boot_info(BootInfo *info) { _boot_info = info; }
    void set_context_factory(ITaskContextFactory *factory) { _task_context_factory = factory; }

    // 核心初始化逻辑
    void setup_infrastructure();
    void spawn_initial_tasks();
    void start_engine();

    void bootstrap();

    IMessageBus *get_message_bus() const { return _bus; }
    ITaskLifecycle *get_task_lifecycle() const { return _lifecycle; }
    TaskService *get_task_service() const { return _task_service; }
    ICPUEngine *get_cpu() const { return _cpu; }
    IAllocator *get_runtime_heap() const { return _runtime_heap; }
    IObjectBuilder *get_object_builder() const { return _builder; }

    ISchedulingStrategy *get_scheduling_strategy() const { return _strategy; }

private:
    KStackBuffer *create_stack(size_t size)
    {
        // 将“去哪里拿内存”和“怎么构建对象”的细节锁死在这里
        return _builder->construct<KStackBuffer>(_runtime_heap, size);
    }

    ITaskControlBlock *create_kernel_task(TaskEntry entry, TaskPriority priority, size_t stack_size, void *config = nullptr);
    void handle_event_print(const Message &msg);

    /**
     * @brief 策略方法：根据当前可用内存决定堆的大小
     */
    size_t calculate_heap_size(size_t preferred_size = 16 * 1024 * 1024) const
    {
        // 获取当前真实的剩余空间
        size_t available = _static_allocator->get_free_size();

        // 策略逻辑：确保堆不会榨干所有内存，预留 20% 给 TCB 和栈对象
        size_t safe_limit = available * 8 / 10;

        return (preferred_size < safe_limit) ? preferred_size : safe_limit;
        ;
    }

    /**
     * @brief 装配方法：执行具体的内存切分和堆对象构造
     */
    IAllocator *create_runtime_heap(size_t size)
    {
        if (size <= sizeof(KernelHeapAllocator))
            return nullptr;

        void *heap_mem = _static_allocator->allocate(size);
        if (!heap_mem)
            return nullptr;

        // 计算管理边界：跳过管理器对象本身占用的空间
        void *actual_managed_start = (uint8_t *)heap_mem + sizeof(KernelHeapAllocator);
        size_t actual_managed_size = size - sizeof(KernelHeapAllocator);

        // 就地构造堆管理器
        return new (heap_mem) KernelHeapAllocator(actual_managed_start, actual_managed_size);
    }
};