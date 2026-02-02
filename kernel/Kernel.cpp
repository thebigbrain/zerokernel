#include <cassert>
#include "Kernel.hpp"
#include "ITaskManager.hpp"
#include "KernelProxy.hpp"
#include "KernelCallback.hpp"
#include <common/BootInfo.hpp>
#include "SimpleTaskManager.hpp"
#include "AsyncSchedulingEngine.hpp"

// 这个宏的作用是：自动生成一个静态的“跳板”并打包成 KernelCallback 对象
#define BIND_KERNEL_CB(Class, Func, ObjPtr)                                      \
    KernelCallback(                                                              \
        [](const Message &m, void *ctx) { static_cast<Class *>(ctx)->Func(m); }, \
        static_cast<void *>(ObjPtr))

void Kernel::bootstrap()
{
    setup_infrastructure();
    spawn_initial_tasks();
    start_engine();
}

void Kernel::setup_infrastructure()
{
    _bus->subscribe(MessageType::SYS_LOAD_TASK, BIND_KERNEL_CB(Kernel, handle_load_task, this));
    _bus->subscribe(MessageType::EVENT_PRINT, BIND_KERNEL_CB(Kernel, handle_event_print, this));
}

void Kernel::spawn_initial_tasks()
{
    // 1. 创建 RootTask。
    // 这里 rt 就是 Kernel 传进来的 KernelProxy 指针，
    // 它将被 TaskManager 填入任务的第一个参数寄存器（如 x64 的 RCX）
    _task_manager->spawn_task(_boot_info->root_task_entry, TaskPriority::ROOT, _boot_info->config_ptr);

    // 2. 创建 IdleTask。
    // Idle 任务通常不需要代理（传入 nullptr），因为它不执行业务 IPC
    _task_manager->spawn_task(_boot_info->idle_task_entry, TaskPriority::IDLE, nullptr);
}

void Kernel::start_engine()
{
    _engine->start();
}

void Kernel::handle_load_task(const Message &msg)
{
    // 委托给管理器执行具体的“加载任务”领域行为
    // 不需要在这里手动算栈顶、调 factory，TaskManager 内部会处理
    _task_manager->spawn_task_from_message(msg);
}

void Kernel::handle_event_print(const Message &msg)
{
    char buf[9] = {0};
    memcpy(buf, &msg.payload[0], 8);
    printf("[Kernel Log] Received from RootTask: %s\n", buf);
}
