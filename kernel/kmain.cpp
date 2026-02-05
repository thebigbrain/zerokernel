/**
 * kmain: 内核入口点
 * @param layout 物理内存布局
 * @param info   启动配置信息
 * @param cpu    物理/模拟 CPU 抽象
 */
#include <new>

#include "Memory.hpp"
#include "Kernel.hpp"
#include "ITaskContextFactory.hpp"
#include "MessageBus.hpp"
#include "StaticLayoutAllocator.hpp"
#include "ISignal.hpp"
#include "PlatformHooks.hpp"

// 定义一个全局指针，用于保存从 kmain 传进来的钩子
static PlatformHooks *g_platform_hooks = nullptr;

// 提供一个内部初始化方法，由 kmain 调用
void kernel_init_platform(PlatformHooks *hooks)
{
    g_platform_hooks = hooks;
}

// 实现一个包装函数，供宏使用
void kernel_panic_handler(const char *msg)
{
    if (g_platform_hooks && g_platform_hooks->panic)
    {
        g_platform_hooks->panic(msg);
    }
    else
    {
        // 如果钩子还没初始化好就崩了，执行最原始的死循环
        // 在真机上这通常意味着通过寄存器直接操作 LED 或串口
        while (true)
            ;
    }
}

extern "C" void kmain(
    PhysicalMemoryLayout layout,
    BootInfo info,
    PlatformHooks *platform_hooks)
{
    // 1. 在物理内存基址处建立引导分配器
    auto static_allocator = StaticLayoutAllocator::create(layout);

    kernel_init_platform(platform_hooks);

    // 2. 利用这个分配器创建第一个，也是永不销毁的对象：Kernel
    // 我们将 loader 传给它，由 Kernel 内部去完成后续的“堆建立”和“Builder建立”
    Kernel *kernel = new (static_allocator->allocate(sizeof(Kernel))) Kernel(static_allocator, info, platform_hooks);

    kernel->bootstrap();
}