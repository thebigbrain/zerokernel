#pragma once

#include "ITaskContext.hpp"
#include "common/TaskTypes.hpp"
#include <common/IUserRuntime.hpp>

/**
 * TCB (Task Control Block) 抽象
 * 它是内核管理任务的实体，只负责状态和上下文，不负责具体的业务逻辑
 */
class ITaskControlBlock
{
public:
    virtual ~ITaskControlBlock() = default;

    virtual uint32_t get_id() const = 0;

    // 状态管理
    virtual TaskState get_state() const = 0;
    virtual void set_state(TaskState state) = 0;

    // 领域逻辑：每个可执行的任务必然关联其执行信息
    // 这样 ExecutionEngine 拿到 ITCB 后，可以直接通过领域模型获取信息
    virtual const TaskExecutionInfo &get_execution_info() const = 0;
    virtual const TaskResourceConfig &get_resource_config() const = 0;

    virtual ITaskContext *get_context() const = 0;

    virtual bool is_queued() const = 0;
    virtual void set_queued(bool queued) = 0;
};