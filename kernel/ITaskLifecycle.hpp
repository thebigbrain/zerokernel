#pragma once

#include <cstdint>
#include <common/Message.hpp>
#include <common/BootInfo.hpp>
#include "ITaskControlBlock.hpp"
#include <common/TaskTypes.hpp>

/**
 * @brief 定义遍历回调签名的抽象接口
 */
class ITaskVisitor
{
public:
    virtual void visit(ITaskControlBlock *tcb) = 0;
};

/**
 * 任务管理器接口：定义了任务的完整生命周期管理行为
 * 它是 Kernel 的一部分，通过依赖倒置提供给 Task 实体使用
 */
class ITaskLifecycle
{
public:
    virtual ~ITaskLifecycle() = default;

    // --- 任务创建 (机制) ---
    // 建议统一入口，移除业务耦合的 spawn_task_from_message (放到业务层或消息处理器)
    virtual ITaskControlBlock *spawn_task(
        const TaskExecutionInfo &exec,
        const TaskResourceConfig &res) = 0;

    // --- 生命周期维护 ---
    virtual void destroy_task(ITaskControlBlock *tcb) = 0;
    virtual void register_task(ITaskControlBlock *tcb) = 0;

    // --- 状态查询 ---
    virtual ITaskControlBlock *get_current_task() const = 0;
    virtual void set_current_task(ITaskControlBlock *tcb) = 0;

    virtual ITaskControlBlock *get_task(uint32_t task_id) = 0;
    virtual size_t get_task_count() const = 0;

    virtual void enumerate_tasks(ITaskVisitor &visitor) const = 0;
};