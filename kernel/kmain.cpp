/**
 * kmain: 内核入口点
 * @param layout 物理内存布局
 * @param info   启动配置信息
 * @param cpu    物理/模拟 CPU 抽象
 */
#include "SimpleTaskManager.hpp"
#include "AsyncSchedulingEngine.hpp"
#include "Memory.hpp"
#include "Kernel.hpp"
#include "KernelProxy.hpp"
#include "SimpleTaskFactory.hpp"
#include "ICPUEngine.hpp"

extern "C" void task_exit_router();

extern "C" void kmain(PhysicalMemoryLayout layout, BootInfo info, ICPUEngine *cpu, ITaskContextFactory *ctx_factory)
{
    // 1. 在内存头部“原地”构建工厂
    // 它是整个内核宇宙的“大爆炸”起点
    ObjectFactory *factory = new (layout.base) ObjectFactory(layout);

    // 2. 预留工厂自身的空间（防止后续分配覆盖自己）
    // 注意：factory 内部其实已经知道 layout.base 被用了，这里显式跳过
    factory->allocate_raw(sizeof(ObjectFactory));

    // 3. 创建核心组件
    // 这些组件现在都受 ObjectFactory 管理，内存可追踪
    ITaskControlBlockFactory *tcb_factory = factory->create<SimpleTaskFactory>(factory, task_exit_router);
    ITaskManager *tm = factory->create<SimpleTaskManager>(factory, ctx_factory, tcb_factory);
    IExecutionEngine *engine = factory->create<AsyncSchedulingEngine>(tm);
    MessageBus *bus = factory->create<MessageBus>(factory);
    IUserRuntime *rt = factory->create<KernelProxy>(bus, tm);

    // 4. 创建内核实例
    Kernel *kernel = factory->create<Kernel>(cpu, factory);

    // 5. 声明式注入依赖
    kernel->set_task_manager(tm);
    kernel->set_execution_engine(engine);
    kernel->set_message_bus(bus);
    kernel->set_user_runtime(rt);

    kernel->set_boot_info(&info);

    // 6. 冷启动：进入 bootstrap 流程
    // 这是一个单向过程，如果是异步引擎，将永不返回
    kernel->bootstrap();
}