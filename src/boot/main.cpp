#include <windows.h>
#include <iostream>
#include "kernel/Kernel.hpp"
#include "arch/win/WinCPUEngine.hpp"
#include "kernel/memory.hpp"

// 模拟任务逻辑
Kernel *g_kernel = nullptr;

// 内核中的全局退出函数
void idle_task_logic()
{
    while (true)
    {
        printf("I am the idle task!\n");
        // 在 Windows 模拟器里可以 Sleep(1) 降低占用
        // 在真机里是 __asm__("hlt");
        Sleep(1);
        g_kernel->yield();
    }
}

// 任务逻辑：现在它们可以通过内核 API 动态产卵
void task_child()
{
    printf("I am a dynamic child task!\n");
    Sleep(1000);
}

void task_parent()
{
    bool spawned = false;
    printf("Parent task: spawning a child once...\n");
    if (!spawned)
    {
        // 在运行中动态创建任务！
        g_kernel->spawn_task(task_child);
        spawned = true;
    }
}

int main()
{
    // --- 1. Bootloader 准备硬件资源 ---
    PhysicalMemoryLayout mem;
    mem.size = 128 * 1024 * 1024;
    mem.base = VirtualAlloc(NULL, mem.size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    WinCPUEngine *winCpu = new WinCPUEngine();

    std::cout << "Bootloader initialized." << std::endl;

    // --- 2. 内核接管 ---
    // 我们把 Factory 也放在线性内存里
    ObjectFactory *factory = new (mem.base) ObjectFactory(mem);
    // 消耗掉 factory 占用的空间
    void *dummy = factory->allocate_raw(sizeof(ObjectFactory));

    std::cout << "Factory initialized." << std::endl;

    g_kernel = factory->create<Kernel>(winCpu, factory);

    std::cout << "Kernel created." << std::endl;

    g_kernel->bootstrap(idle_task_logic);

    std::cout << "Kernel ready." << std::endl;

    // --- 加载用户任务 ---
    g_kernel->spawn_task(task_parent);

    // --- 启动执行引擎 ---
    // 启动 0 号任务 (Idle)
    Task *first = g_kernel->get_ready_task(0);
    if (first)
    {
        winCpu->execute(first->get_context());
    }

    std::cout << "Kernel terminated." << std::endl;

    return 0;
}
