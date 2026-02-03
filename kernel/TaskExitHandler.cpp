#include "ISchedulingControl.hpp"

// 使用静态全局变量，将其可见性限制在本文件内
static ISchedulingControl *g_active_scheduler = nullptr;

/**
 * 这是一个系统级的“挂钩”方法
 * 由 Kernel 在初始化 TaskManager 时调用
 */
extern "C" void set_active_scheduler(ISchedulingControl *mgr)
{
    g_active_scheduler = mgr;
}

extern "C" void cpu_halt();

/**
 * 物理路由：当任务的 main 函数返回时，CPU 会通过栈上的返回地址跳到这里
 */
extern "C" void task_exit_router()
{
    if (g_active_scheduler)
    {
        // 委托给当前的管理器进行逻辑收割
        g_active_scheduler->terminate_current_task();
    }
    else
    {
        // 如果没有管理器，说明内核环境异常，强制挂起 CPU
        cpu_halt();
    }
}