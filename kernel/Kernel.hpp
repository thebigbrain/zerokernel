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
    ObjectFactory *_factory;
    ICPUEngine *_cpu;

    // 领域组件 (组合优于继承)
    ITaskManager *_task_manager;
    IExecutionEngine *_engine;
    MessageBus *_bus;

    BootInfo *_boot_info;

public:
    Kernel(ICPUEngine *cpu, ObjectFactory *factory)
        : _cpu(cpu), _factory(factory)
    {
    }

    // 设置执行策略（同步测试引擎 or 异步调度引擎）
    void set_execution_engine(IExecutionEngine *engine) { _engine = engine; }

    // 设置任务管理器实现
    void set_task_manager(ITaskManager *tm) { _task_manager = tm; }

    void bootstrap(BootInfo *info);

    // 领域逻辑：处理系统事件
    void handle_load_task(const Message &msg);
    void handle_event_print(const Message &msg);

    // 暴露给外部或 Proxy 的接口，转发给 TaskManager
    ITaskManager *get_task_manager() { return _task_manager; }
};