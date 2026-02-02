#pragma once

#include <cstdint>
#include <common/Message.hpp>
#include <common/BootInfo.hpp>
#include "ITaskControlBlock.hpp"
#include <common/TaskTypes.hpp>

/**
 * 任务管理器接口：定义了任务的完整生命周期管理行为
 * 它是 Kernel 的一部分，通过依赖倒置提供给 Task 实体使用
 */
class ITaskManager
{
public:
    virtual ~ITaskManager() = default;

    /**
     * 原先的 spawn_task：创建一个纯粹的任务实体
     * 职责：分配 TCB、栈、初始化上下文，并将其加入就绪队列
     */
    virtual ITaskControlBlock *spawn_task(
        void *entry,
        TaskPriority priority = TaskPriority::NORMAL,
        void *config = nullptr) = 0;

    /**
     * 业务接口：从内核消息解析意图并创建任务
     */
    virtual void spawn_task_from_message(const Message &msg) = 0;

    virtual ITaskControlBlock *get_current_task() const = 0;
    virtual size_t get_task_count() const = 0;

    /**
     * 核心退出行为：由当前正在运行的任务通过特定路由调用
     * 实现者应负责：
     * 1. 标记当前任务为 DEAD 状态
     * 2. 释放任务占用的非共享资源（如私有栈、上下文对象）
     * 3. 触发 ExecutionEngine 切换到下一个就绪任务
     */
    virtual void terminate_current_task() = 0;

    /**
     * 任务注册：将创建好的控制块纳入管理器追踪范围
     */
    virtual void register_task(ITaskControlBlock *tcb) = 0;

    /**
     * 获取任务实例：返回通用控制接口
     */
    virtual ITaskControlBlock *get_task(uint32_t task_id) = 0;

    /**
     * 协作式调度：由 Task 主动让出执行权
     */
    virtual void yield_current_task() = 0;

    /**
     * 核心调度决策接口
     * 职责：根据调度算法（如 RR, FIFO）从就绪队列中弹出一个任务
     * 返回值：下一个要运行的任务实体
     */
    virtual ITaskControlBlock *pick_next_ready_task() = 0;

    /**
     * 状态更新接口：当任务让出 CPU 或被抢占时，将其重新放回队列
     */
    virtual void make_task_ready(ITaskControlBlock *tcb) = 0;
};