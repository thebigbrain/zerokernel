#include <windows.h>
#include <iostream>
#include "kernel/Kernel.hpp"
#include "arch/win/WinCPUEngine.hpp"
#include "kernel/memory.hpp"

// 模拟任务逻辑
void task_a_entry()
{
    extern Kernel *g_kernel;
    while (true)
    {
        std::cout << "[CPU] Running Task A..." << std::endl;
        Sleep(500);
        g_kernel->yield();
    }
}

void task_b_entry()
{
    extern Kernel *g_kernel;
    while (true)
    {
        std::cout << "[CPU] Running Task B..." << std::endl;
        Sleep(500);
        g_kernel->yield();
    }
}

Kernel *g_kernel = nullptr;

int main()
{
    // 1. 准备线性内存 (内核领土)
    PhysicalMemoryLayout layout;
    layout.size = 64 * 1024 * 1024;
    layout.base = VirtualAlloc(NULL, layout.size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    // 2. 准备执行器
    WinCPUEngine *cpu = new WinCPUEngine();

    // 3. 引导内核 (Placement New 在线性内存上构造)
    g_kernel = new (layout.base) Kernel(cpu);

    // 4. 利用内核分配器构造任务 (这里简化为手动计算偏移)
    uint8_t *pool = (uint8_t *)layout.base + sizeof(Kernel);

    // 创建 Context 对象
    WinTaskContext *ctx1 = new (pool) WinTaskContext();
    pool += sizeof(WinTaskContext);
    WinTaskContext *ctx2 = new (pool) WinTaskContext();
    pool += sizeof(WinTaskContext);

    // 创建 Task 对象
    Task *t1 = new (pool) Task(1, ctx1);
    pool += sizeof(Task);
    Task *t2 = new (pool) Task(2, ctx2);
    pool += sizeof(Task);

    // 分配栈
    void *stack1 = pool + 16384;
    pool += 16384;
    void *stack2 = pool + 16384;
    pool += 16384;

    t1->prepare(task_a_entry, stack1);
    t2->prepare(task_b_entry, stack2);

    g_kernel->set_tasks(t1, t2);

    std::cout << "Kernel bootstrapped. Handing over to CPU Engine..." << std::endl;

    // 5. 启动执行引擎
    cpu->execute(t1->get_context());

    return 0;
}