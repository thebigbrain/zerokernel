#include "Kernel.hpp"
#include "ITaskManager.hpp"

static ITaskManager *g_manager_instance = nullptr;

// 提供给外部注入实例的方法
void set_task_manager_instance(ITaskManager *mgr)
{
    g_manager_instance = mgr;
}

// 汇编 ret 指令跳转到这里
extern "C" void task_exit_router()
{
    if (g_manager_instance)
    {
        g_manager_instance->terminate_current_task();
    }
    // 如果没有管理器，系统可能需要 halt
}

void Kernel::bootstrap(void (*idle_logic)())
{
    set_task_manager_instance(this);
    // 显式创建 0 号任务
    this->spawn_task(idle_logic);
}