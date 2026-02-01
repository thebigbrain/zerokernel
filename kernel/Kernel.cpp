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

void Kernel::bootstrap(BootInfo *info)
{
    _boot_info = info;

    // 1. 基础设施初始化 (通过组合持有的对象)
    this->_bus = _factory->create<MessageBus>(_factory);

    // 2. 领域逻辑订阅
    _bus->subscribe(MessageType::SYS_LOAD_TASK, BIND_KERNEL_CB(Kernel, handle_load_task, this));
    _bus->subscribe(MessageType::EVENT_PRINT, BIND_KERNEL_CB(Kernel, handle_event_print, this));

    // 3. 移交控制权给引擎
    // 注意：我们将 KernelProxy 的创建和注入也视为一种启动策略，交给引擎或管理器
    KernelProxy proxy(_bus, this->get_task_manager());
    _engine->start(_boot_info->root_task_entry, _boot_info->config_ptr, &proxy);
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
