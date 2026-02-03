#pragma once

#include "ITaskLifecycle.hpp"
#include "ISchedulingStrategy.hpp"
#include "IMessageBus.hpp"
#include "Defs.hpp"

class TaskService
{
private:
    ITaskLifecycle *_lifecycle;     // 负责“生”和“死”
    ISchedulingStrategy *_strategy; // 负责“在哪排队”
    IMessageBus *_message_bus;      // 负责“沟通”

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
     * 业务处理逻辑：从消息解析意图
     */
    void TaskService::handle_spawn_request(const Message &msg)
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