#pragma once

#include "ITaskControlBlock.hpp"

/**
 * TaskControlBlock: ITaskControlBlock 的标准实现
 * 负责持有任务运行所需的元数据、上下文句柄以及通信信箱
 */
class TaskControlBlock : public ITaskControlBlock
{
private:
    // 1. 基础属性
    uint32_t _id;
    TaskState _state;

    // 2. 硬件资源上下文 (注入的执行机制)
    ITaskContext *_context;

    // 3. 领域模型 (直接组合)
    TaskExecutionInfo _exec_info;   // 执行意图：去哪跑，带什么参数
    TaskResourceConfig _res_config; // 资源约束：优先级，栈大小

public:
    TaskControlBlock(
        uint32_t id,
        ITaskContext *ctx,
        const TaskExecutionInfo &exec_info,
        const TaskResourceConfig &res_config) : _id(id),
                                                _context(ctx),
                                                _state(TaskState::READY),
                                                _exec_info(exec_info),
                                                _res_config(res_config) {}

    // 实现接口：获取执行信息
    const TaskExecutionInfo &get_execution_info() const override
    {
        return _exec_info;
    }

    // 实现接口：获取资源配置
    const TaskResourceConfig &get_resource_config() const override
    {
        return _res_config;
    }

    ITaskContext *get_context() const override { return _context; }

    uint32_t get_id() const override { return _id; }
    TaskState get_state() const override { return _state; }

    void set_state(TaskState state) override { _state = state; }
};