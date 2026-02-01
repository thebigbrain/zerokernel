#pragma once

#include <common/IUserRuntime.hpp>
#include "ITaskControlBlock.hpp"

class IExecutionEngine
{
public:
    // 启动初始任务并接管后续的执行流
    virtual void start(void *entry, void *arg, IUserRuntime *rt) = 0;

    // 内核通过此接口通知引擎：有一个新任务可以被调度了
    virtual void schedule_task(ITaskControlBlock *task) = 0;

    virtual ~IExecutionEngine() = default;
};