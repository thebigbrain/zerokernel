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
    size_t heap_size = 16 * 1024 * 1024;
    void *heap_mem = _static_allocator->allocate(heap_size);

    // 关键修复：堆管理器只管理除去自身大小之后的空间
    // 假设 KernelHeapAllocator 内部逻辑是从 base 开始管理
    void *actual_managed_start = (uint8_t *)heap_mem + sizeof(KernelHeapAllocator);
    size_t actual_managed_size = heap_size - sizeof(KernelHeapAllocator);

    _runtime_heap = new (heap_mem) KernelHeapAllocator(actual_managed_start, actual_managed_size);

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
    // --- 1. 创建 RootTask ---
    TaskExecutionInfo root_exec;
    root_exec.entry = reinterpret_cast<TaskEntry>(_boot_info->root_task_entry);
    root_exec.config = _boot_info->config_ptr;
    root_exec.runtime = nullptr; // 之后由 Proxy 注入

    TaskResourceConfig root_res;
    root_res.priority = TaskPriority::ROOT;
    // 【关键修复】：创建对象化的栈空间，而不是只给一个数字
    root_res.stack = create_stack(32768);

    ITaskControlBlock *root_tcb = _lifecycle->spawn_task(root_exec, root_res);
    if (root_tcb && _strategy)
    {
        _strategy->make_task_ready(root_tcb);
    }

    // --- 2. 创建 IdleTask ---
    TaskExecutionInfo idle_exec;
    idle_exec.entry = reinterpret_cast<TaskEntry>(_boot_info->idle_task_entry);

    TaskResourceConfig idle_res;
    idle_res.priority = TaskPriority::IDLE;
    // 【关键修复】：Idle 任务同样需要对象化的栈
    idle_res.stack = create_stack(4096);

    ITaskControlBlock *idle_tcb = _lifecycle->spawn_task(idle_exec, idle_res);
    if (idle_tcb && _strategy)
    {
        _strategy->make_task_ready(idle_tcb);
    }
}

void Kernel::start_engine()
{
    if (_engine)
    {
        _engine->start();
    }
}

void Kernel::handle_event_print(const Message &msg)
{
    // 简单的日志处理逻辑
    printf("[Kernel Log] Received Message Type: %d\n", static_cast<int>(msg.type));
}