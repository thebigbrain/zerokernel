#pragma once

#include "common/BootInfo.hpp"
#include "ITaskManager.hpp"
#include "IExecutionEngine.hpp"
#include "MessageBus.hpp"
#include "ObjectFactory.hpp"
#include "ICPUEngine.hpp"

class Kernel
{
private:
    // 基础依赖
    ObjectFactory *_obj_factory;
    ICPUEngine *_cpu;

    // 领域组件 (组合优于继承)
    ITaskManager *_task_manager;
    IExecutionEngine *_engine;
    MessageBus *_bus;

    BootInfo *_boot_info;

    IUserRuntime *_user_runtime;

public:
    Kernel(ICPUEngine *cpu, ObjectFactory *factory)
        : _cpu(cpu), _obj_factory(factory)
    {
    }

    void set_boot_info(BootInfo *info) { _boot_info = info; }

    // 设置执行策略（同步测试引擎 or 异步调度引擎）
    void set_execution_engine(IExecutionEngine *engine) { _engine = engine; }

    // 设置任务管理器实现
    void set_task_manager(ITaskManager *tm) { _task_manager = tm; }

    void set_message_bus(MessageBus *bus) { _bus = bus; }

    void set_user_runtime(IUserRuntime *runtime) { _user_runtime = runtime; }

    void setup_infrastructure(); // 仅仅建立总线连接
    void spawn_initial_tasks();  // 仅仅创建 TCB，但不运行
    void start_engine();         // 移交控制权

    // 原有的 bootstrap 只是这些动作的顺序组合
    void bootstrap();

private:
    // 领域逻辑：处理系统事件
    void handle_load_task(const Message &msg);
    void handle_event_print(const Message &msg);
};