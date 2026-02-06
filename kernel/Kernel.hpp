#pragma once

#include <new>
#include <cstdio>

#include "common/BootInfo.hpp"

#include "common/diagnostics.hpp"
#include "PlatformHooks.hpp"
#include "StaticLayoutAllocator.hpp"

#include "KList.hpp"
#include "ISignal.hpp"

#include "TaskService.hpp"
#include "IMessageBus.hpp"
#include "IObjectBuilder.hpp" // 引入 Builder 接口
#include "ITaskControlBlockFactory.hpp"
#include "ITaskContextFactory.hpp"
#include "KStackBuffer.hpp"
#include "ISchedulingControl.hpp"
#include "KernelHeapAllocator.hpp"

#include "RoundRobinStrategy.hpp"
#include "SimpleTaskLifecycle.hpp"
#include "KernelObjectBuilder.hpp"
#include "MessageBus.hpp"
#include "BitmapIdGenerator.hpp"
#include "SimpleTaskFactory.hpp"

#include "KernelProxy.hpp"

/**
 * @brief 任务档案：存储任务的静态元数据，不随任务状态改变
 */
struct TaskArchive
{
    uint32_t id;            // 任务唯一ID
    TaskEntry entry;        // 任务入口函数指针
    TaskPriority priority;  // 初始优先级
    const char *name;       // 任务名称（调试友好）
    ITaskControlBlock *tcb; // 对应的控制块指针
};

class Kernel : public ISignalListener
{
private:
    friend class KernelInspector;

    // 基础依赖
    StaticLayoutAllocator *_static_allocator; // 初始静态分配器
    IAllocator *_runtime_heap;                // 稍后建立的动态堆
    IObjectBuilder *_builder;                 // 稍后建立的业务构建器

    ITaskControlBlockFactory *_tcb_factory;

    // 领域组件
    TaskService *_task_service;
    IMessageBus *_bus;
    ITaskLifecycle *_lifecycle;
    ISchedulingStrategy *_strategy;

    BootInfo &_boot_info;
    IUserRuntime *_user_runtime = nullptr;
    // 任务注册表：让每一个进入内核的任务都有据可查
    KList<TaskArchive> *_task_archives = nullptr;

    PlatformHooks *_platform_hooks = nullptr;

    ITaskControlBlock *_idle_tcb = nullptr;

public:
    // 构造函数：注入 Builder 和 CPU 引擎
    Kernel(
        StaticLayoutAllocator *static_allocator,
        BootInfo &info,
        PlatformHooks *hooks)
        : _static_allocator(static_allocator),
          _boot_info(info),
          _platform_hooks(hooks),

          _runtime_heap(nullptr), // 必须手动清零
          _builder(nullptr),
          _task_service(nullptr),
          _bus(nullptr),
          _lifecycle(nullptr),
          _strategy(nullptr)
    {
    }

    void bootstrap()
    {
        setup_infrastructure();
        setup_boot_tasks();
        start_engine();
    }

    // 核心初始化逻辑
    void setup_infrastructure()
    {
        // 1. 建立运行时堆 (从静态分配器中划拨 128MB)
        size_t heap_size = calculate_heap_size(16 * 1024 * 1024);

        _runtime_heap = create_runtime_heap(heap_size);

        // 2. 建立业务构建器 (从静态分配器中划拨 Builder 所需空间)
        void *builder_mem = _static_allocator->allocate(sizeof(KernelObjectBuilder));
        // Builder 将使用刚刚建立的 _runtime_heap 作为其分配源
        _builder = new (builder_mem) KernelObjectBuilder(_runtime_heap);

        // 所有的组件现在都统一收纳在 Kernel 内部
        _bus = _builder->construct<MessageBus>(_builder);

        _bus->subscribe(MessageType::EVENT_PRINT, BIND_MESSAGE_CB(Kernel, handle_event_print, this));

        auto id_gen = _builder->construct<BitmapIdGenerator<64>>();
        // 注入 builder 即可，Factory 内部需要资源时，Kernel 会提供辅助
        _tcb_factory = _builder->construct<SimpleTaskFactory>(_builder, _platform_hooks->task_context_factory, id_gen);

        _strategy = _builder->construct<RoundRobinStrategy>(_builder);
        _lifecycle = _builder->construct<SimpleTaskLifecycle>(_builder, _tcb_factory);

        // 组装 Service
        _task_service = _builder->construct<TaskService>(_lifecycle, _strategy, _bus);

        _task_archives = _builder->construct<KList<TaskArchive>>(_builder);
    }

    void setup_boot_tasks()
    {
        ITaskControlBlock *root_tcb = create_kernel_task(_boot_info.root_task_entry, TaskPriority::ROOT, 4096, nullptr, "RootTask");
        _idle_tcb = create_kernel_task(Kernel::static_idle_entry, TaskPriority::IDLE, 1024, this, "IdleTask");

        // 2. 缝合到 TaskService
        _task_service->bind_root_task(root_tcb);

        K_ASSERT(_idle_tcb->get_context() != nullptr, "Idle context not initialized");
        K_ASSERT(root_tcb->get_context() != nullptr, "RootTask context missing");
    }

    /**
     * @brief 启动引擎：完成内核从“主动控制”到“被动监听”的身份转换
     * * 此函数是内核引导（Bootstrap）的最后一步。
     * 执行完成后，当前执行流将彻底消失，控制权移交给 RootTask。
     */
    void start_engine()
    {
        K_INFO("Start Engine ...");
        // 1. 获取预先静态分配好的 RootTask
        // RootTask 及其 TaskContext 在 setup_infrastructure() 中已完成静态绑定
        ITaskControlBlock *root_tcb = _task_service->get_root_task();

        if (root_tcb == nullptr)
        {
            // 如果没有 RootTask，内核将无处可去，必须触发 Panic
            K_PANIC("Start Engine Failed: RootTask archive is missing.");
        }

        // 2. 建立监听契约：挂载内核监听入口
        // 告诉信号分发者（ISignalDispatcher）：
        // “从现在起，无论谁在执行，只要发生信号，就调用我的 on_signal_received 接口”
        _platform_hooks->dispatcher->bind_listener(this);

        // 3. 激活物理/模拟分发线路
        // 在硬件上：这通常对应开启全局中断（如 ARM 的 cpsie i）
        // 在 Mock 上：这对应启动模拟器的时序产生逻辑
        _platform_hooks->dispatcher->activate();

        // 4. 执行物理跳转（不归路）
        // 从 RootTask 的 Archive 中提取初始上下文（SP, PC, Registers）并覆盖当前 CPU 状态
        // 此行代码执行后，CPU 将跳转到 RootTask 的入口点执行

        _idle_tcb->get_context()->transit_to(root_tcb->get_context());

        // --- 逻辑真空区 ---
        // 正常情况下，CPU 永远不会执行到这里。
        // 如果执行到了，说明硬件跳转或上下文恢复发生了致命错误。
        K_PANIC("Kernel Control Breach: Execution flow returned from RootTask.");
    }

    /**
     * @brief 实现 ISignalListener 接口
     * 所有来自硬件或 Mock 的分发最终都会汇聚到这里
     */
    void on_signal_received(SignalPacket packet) override
    {
        // 这里是内核在启动后的“复活点”
        this->dispatch_logic(packet);
    }

private:
    void dispatch_logic(SignalPacket &packet)
    {
        // 后续的路由决策中心
        // switch(packet.type) { ... }
    }

    ITaskControlBlock *create_kernel_task(TaskEntry entry, TaskPriority priority, size_t stack_size, void *config = nullptr, const char *name = "k_service_unamed")
    {
        TaskExecutionInfo exec{};
        exec.entry = entry;
        exec.runtime = _builder->construct<KernelProxy>(_bus, _platform_hooks->sched_control);
        exec.config = config;

        TaskResourceConfig res{};
        res.priority = priority;
        res.stack = _builder->construct<KStackBuffer>(_runtime_heap, stack_size);

        // 防御性检查：避免后续访问 root_res.stack->get_base() 时崩于 0x0
        if (res.stack == nullptr)
        {
            return nullptr;
        }

        ITaskControlBlock *tcb = _lifecycle->spawn_task(exec, res);
        if (tcb)
        {
            _strategy->make_task_ready(tcb);
        }

        return tcb;
    }

    void handle_event_print(const Message &msg)
    {
        // 简单的日志处理逻辑
        printf("[Kernel Log] Received Message Type: %d\n", static_cast<int>(msg.type));
    }

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

private:
    // 1. 实际的逻辑函数
    void idle_task_logic()
    {
        while (true)
        {
            if (_platform_hooks && _platform_hooks->halt)
                _platform_hooks->halt();

            K_DEBUG("Idle Task Running ...");
        }
    }

    // 2. 静态中转：必须符合 (runtime, config) 的顺序
    static void static_idle_entry(void *runtime, void *config)
    {
        // 根据你的设计，config 承载了 Kernel 的 this 指针
        Kernel *self = static_cast<Kernel *>(config);
        if (self)
        {
            self->idle_task_logic();
        }
    }
};