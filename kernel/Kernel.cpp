#include <cassert>
#include <cstdio>
#include <cstring> // 修复原代码中 memcpy 未包含头文件的问题
#include <new>

#include "Kernel.hpp"
#include "KernelProxy.hpp"
#include "MessageCallback.hpp"
#include "KStackBuffer.hpp"
#include "SimpleTaskFactory.hpp" // 假设具体工厂实现
#include "SimpleTaskLifecycle.hpp"
#include <common/BootInfo.hpp>
#include "MessageBus.hpp"
#include "RoundRobinStrategy.hpp"
#include "KernelHeapAllocator.hpp"
#include "KernelObjectBuilder.hpp"
#include "AsyncSchedulingEngine.hpp"
#include "SimpleSchedulingControl.hpp"

extern "C" void task_exit_router();

void Kernel::bootstrap()
{
    // 基础设施组装 (MessageBus, TaskService, etc.)
    setup_infrastructure();

    // 业务启动
    spawn_initial_tasks();

    // 引擎运转
    start_engine();
}

void Kernel::setup_infrastructure()
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
    _tcb_factory = _builder->construct<SimpleTaskFactory>(_builder, _task_context_factory, id_gen, task_exit_router);

    _strategy = _builder->construct<RoundRobinStrategy>(_builder);
    _lifecycle = _builder->construct<SimpleTaskLifecycle>(_builder, _tcb_factory);
    _scheduling_control = _builder->construct<SimpleSchedulingControl>(_lifecycle);

    // 组装 Service
    _task_service = _builder->construct<TaskService>(_lifecycle, _strategy, _bus);

    _engine = _builder->construct<AsyncSchedulingEngine>(_lifecycle, _scheduling_control, _strategy);
}

void Kernel::spawn_initial_tasks()
{
    // 1. 启动根任务 (系统的灵魂)
    ITaskControlBlock *root = create_kernel_task(
        (TaskEntry)_boot_info->root_task_entry,
        TaskPriority::ROOT,
        32768, // 32KB Stack
        _boot_info->config_ptr);

    // 2. 启动空闲任务 (系统的底座)
    ITaskControlBlock *idle = create_kernel_task(
        (TaskEntry)_boot_info->idle_task_entry,
        TaskPriority::IDLE,
        4096 // 4KB Stack
    );
}

void Kernel::start_engine()
{
    if (_engine)
    {
        _engine->start();
    }
}

ITaskControlBlock *Kernel::create_kernel_task(TaskEntry entry, TaskPriority priority, size_t stack_size, void *config)
{
    TaskExecutionInfo exec{};
    exec.entry = entry;
    exec.config = config;
    // exec.runtime 留空，由之后 Proxy 注入 KObject 基础类逻辑

    TaskResourceConfig res{};
    res.priority = priority;
    res.stack = create_stack(stack_size);

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

void Kernel::handle_event_print(const Message &msg)
{
    // 简单的日志处理逻辑
    printf("[Kernel Log] Received Message Type: %d\n", static_cast<int>(msg.type));
}