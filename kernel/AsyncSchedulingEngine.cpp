#include "AsyncSchedulingEngine.hpp"
#include "ITaskControlBlock.hpp"

void AsyncSchedulingEngine::start(void *entry, void *arg, IUserRuntime *rt)
{
    // 1. 创建 RootTask。
    // 这里 rt 就是 Kernel 传进来的 KernelProxy 指针，
    // 它将被 TaskManager 填入任务的第一个参数寄存器（如 x64 的 RCX）
    _tm->spawn_fixed_task(entry, arg, rt);

    // 2. 创建 IdleTask。
    // Idle 任务通常不需要代理（传入 nullptr），因为它不执行业务 IPC
    void *idle_entry = _tm->get_boot_info()->idle_task_entry;
    _tm->spawn_fixed_task(idle_entry, nullptr, nullptr);

    // 3. 进入硬件级别的调度循环
    this->enter_hardware_run_loop();
}

void AsyncSchedulingEngine::enter_hardware_run_loop()
{
    while (true)
    {
        // 领域决策：选谁跑？
        ITaskControlBlock *next = _tm->pick_next_ready_task();

        if (next)
        {
            // 物理行为：执行上下文切换
            // 注意：这里一旦 switch_to，当前的执行流就挂起了
            // 直到中断发生或任务主动 yield 返回到调度器
            _cpu->switch_to(next->get_context());
        }
    }
}