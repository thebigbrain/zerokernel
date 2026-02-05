#pragma once

#include "ITaskLifecycle.hpp"
#include "ISchedulingStrategy.hpp"
#include "IMessageBus.hpp"
#include "MessageCallback.hpp"

class TaskService
{
private:
    ITaskLifecycle *_lifecycle;     // 负责“生”和“死”
    ISchedulingStrategy *_strategy; // 负责“在哪排队”
    IMessageBus *_message_bus;      // 负责“沟通”

    ITaskControlBlock *_root_task = nullptr;
    ITaskControlBlock *_idle_task = nullptr;

public:
    TaskService(ITaskLifecycle *lifecycle,
                ISchedulingStrategy *strategy,
                IMessageBus *bus)
        : _lifecycle(lifecycle), _strategy(strategy), _message_bus(bus)
    {
        // 初始化时订阅任务创建请求
        _message_bus->subscribe(MessageType::SYS_LOAD_TASK, BIND_MESSAGE_CB(TaskService, handle_spawn_request, this));
    }

    /**
     * @brief 绑定系统核心任务
     * 由 Kernel 在引导期间调用，传入已经静态分配好的 TCB
     */
    void bind_core_tasks(ITaskControlBlock *root, ITaskControlBlock *idle)
    {
        _root_task = root;
        _idle_task = idle;

        // 系统任务通常也要进入调度策略，以便在没有业务任务时切换到 Idle
        if (_root_task)
            _strategy->make_task_ready(_root_task);
        if (_idle_task)
            _strategy->make_task_ready(_idle_task);
    }

    ITaskControlBlock *get_root_task() const { return _root_task; }
    ITaskControlBlock *get_idle_task() const { return _idle_task; }

    /**
     * @brief 遍历系统中所有存在的任务（包括就绪、阻塞或运行中）
     */
    void inspect_all_tasks(ITaskVisitor &visitor) const
    {
        if (_lifecycle)
        {
            _lifecycle->enumerate_tasks(visitor);
        }
    }

    /**
     * 业务处理逻辑：从消息解析意图
     */
    void handle_spawn_request(const Message &msg)
    {
        auto *params = reinterpret_cast<const TaskSpawnParams *>(msg.payload);

        // 1. 调用 Lifecycle 创建物理实体
        // 我们可以直接把整个 params 传进去，或者拆开传
        ITaskControlBlock *tcb = _lifecycle->spawn_task(
            params->exec_info,
            params->res_config);

        if (tcb)
        {
            // 2. 放入调度器
            _strategy->make_task_ready(tcb);
        }
    }
    /**
     * 优雅退出业务
     */
    void kill_task_by_id(uint32_t task_id)
    {
        if (task_id == _root_task->get_id())
            return;

        ITaskControlBlock *tcb = _lifecycle->get_task(task_id);
        if (tcb)
        {
            // 从调度算法中移除
            _strategy->remove_task(tcb);
            // 回收资源
            _lifecycle->destroy_task(tcb);
        }
    }
};