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
private:
    char _name[32];

public:
    virtual ~ITaskControlBlock() = default;

    virtual uint32_t get_id() const = 0;

    virtual const char *get_name() const
    {
        return _name;
    }

    virtual void set_name(const char *name)
    {
        // 使用安全的拷贝函数，确保不越界
        // Windows 下可用 strncpy_s，通用可用 strncpy
        std::strncpy(_name, name, sizeof(_name) - 1);
        _name[sizeof(_name) - 1] = '\0'; // 强制结尾
    }

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