/**
 * kmain: 内核入口点
 * @param layout 物理内存布局
 * @param info   启动配置信息
 * @param cpu    物理/模拟 CPU 抽象
 */
#include <new>

#include "TaskService.hpp"
#include "AsyncSchedulingEngine.hpp"
#include "Memory.hpp"
#include "Kernel.hpp"
#include "KernelProxy.hpp"
#include "SimpleTaskFactory.hpp"
#include "ICPUEngine.hpp"
#include "ITaskContextFactory.hpp"
#include "RoundRobinStrategy.hpp"
#include "MessageBus.hpp"
#include "SimpleSchedulingControl.hpp"
#include "SimpleTaskLifecycle.hpp"
#include "GenericObjectBuilder.hpp"
#include "KernelHeapAllocator.hpp"
#include "StaticLayoutAllocator.hpp"
#include "KernelObjectBuilder.hpp"

extern "C" void kmain(PhysicalMemoryLayout layout, BootInfo info, ICPUEngine *cpu, ITaskContextFactory *ctx_factory)
{
    // 1. 在物理内存基址处建立引导分配器
    auto static_allocator = StaticLayoutAllocator::create(layout);

    // 2. 利用这个分配器创建第一个，也是永不销毁的对象：Kernel
    // 我们将 loader 传给它，由 Kernel 内部去完成后续的“堆建立”和“Builder建立”
    Kernel *kernel = new (static_allocator->allocate(sizeof(Kernel))) Kernel(static_allocator, cpu);

    // 3. 注入必要信息并启动
    kernel->set_boot_info(&info);
    kernel->set_context_factory(ctx_factory);

    kernel->bootstrap();
}