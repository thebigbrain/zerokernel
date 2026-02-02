#pragma once

#include "SimpleTaskFactory.hpp"
#include "TaskControlBlock.hpp"

ITaskControlBlock *SimpleTaskFactory::create_tcb(
    uint32_t id,
    ITaskContext *ctx,
    const TaskExecutionInfo &exec_info,
    const TaskResourceConfig &res_config)
{
    void *stack_base = _obj_factory->allocate_raw(res_config.stack_size);
    void *stack_top = static_cast<uint8_t *>(stack_base) + res_config.stack_size;

    // 使用你定义的语义化接口进行初始化
    ctx->setup_flow(
        reinterpret_cast<void (*)()>(exec_info.entry),
        stack_top,
        reinterpret_cast<void (*)()>(_default_exit_router));

    // 设置初始参数（这里体现了任务与上下文的结合）
    ctx->load_argument(0, reinterpret_cast<uintptr_t>(exec_info.config));

    // 最终产生 TCB
    return _obj_factory->create<TaskControlBlock>(id, ctx, exec_info, res_config);
}