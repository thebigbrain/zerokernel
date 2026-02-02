#pragma once

#include "ITaskControlBlock.hpp"
#include "common/TaskTypes.hpp"
#include "ITaskContext.hpp"

/**
 * ITaskControlBlockFactory: 负责将领域信息转化为具体的 TCB 实例
 */
class ITaskControlBlockFactory
{
public:
    virtual ITaskControlBlock *create_tcb(
        uint32_t id,
        ITaskContext *ctx,
        const TaskExecutionInfo &exec_info,
        const TaskResourceConfig &res_config) = 0;

    virtual ~ITaskControlBlockFactory() = default;
};