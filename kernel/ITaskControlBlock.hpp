#pragma once

#include "ITaskContext.hpp"
#include "TaskState.hpp"
#include "Mailbox.hpp"

/**
 * TCB (Task Control Block) 抽象
 * 它是内核管理任务的实体，只负责状态和上下文，不负责具体的业务逻辑
 */
class ITaskControlBlock
{
public:
    virtual ~ITaskControlBlock() = default;

    virtual uint32_t get_id() const = 0;

    // 获取/设置该任务的物理上下文
    virtual ITaskContext *get_context() = 0;

    // 状态管理
    virtual void set_state(TaskState state) = 0;
    virtual TaskState get_state() const = 0;

    // 只有当任务是“活动对象”时才需要这个，但在微内核中，
    // 我们更倾向于通过 MessageBus 直接投递到任务的邮箱，
    // 而不是让 TCB 暴露这些方法。
    virtual Mailbox *get_mailbox() = 0;

    /**
     * 领域逻辑：向任务投递消息
     * 这个方法是 MessageBus 最终调用的地方
     */
    void deliver(const Message &msg)
    {
        if (get_mailbox()->push(msg))
        {
            // 如果任务处于 BLOCKED 状态，投递后应由 ITaskManager 唤醒
            if (get_state() == TaskState::BLOCKED)
            {
                set_state(TaskState::READY);
            }
        }
    }
};