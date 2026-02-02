#pragma once
#include "ITaskManager.hpp"
#include "ObjectFactory.hpp"
#include "ITaskContextFactory.hpp"
#include "ITaskControlBlock.hpp"
#include "TaskControlBlock.hpp" // 必须包含具体实现
#include <queue>                // 补全 queue

#include "ITaskControlBlockFactory.hpp"

class SimpleTaskManager : public ITaskManager
{
private:
    ObjectFactory *_obj_factory;
    ITaskContextFactory *_context_factory;
    std::queue<ITaskControlBlock *> _ready_queue;

    // 存储所有 TCB 的容器，用于 get_task_count
    // 在嵌入式环境下，通常使用静态数组或在 ObjectFactory 分配的链表
    static const size_t MAX_TASKS = 64;
    ITaskControlBlock *_tasks[MAX_TASKS];
    size_t _task_count = 0;

    ITaskControlBlockFactory *_tcb_factory;

    // 记录当前正在运行的任务，用于 get_current_task
    ITaskControlBlock *_current_task = nullptr;

public:
    SimpleTaskManager(ObjectFactory *f, ITaskContextFactory *tcf, ITaskControlBlockFactory *tf);
    ~SimpleTaskManager() override;

    ITaskControlBlock *get_current_task() const override { return _current_task; }
    size_t get_task_count() const override { return _task_count; }

    // 接口实现
    ITaskControlBlock *spawn_task(void *entry,
                                  TaskPriority priority = TaskPriority::NORMAL,
                                  void *config = nullptr) override; // 修正返回指针

    void spawn_task_from_message(const Message &msg) override;

    void terminate_current_task() override;
    void yield_current_task() override;

    ITaskControlBlock *pick_next_ready_task() override;
    void make_task_ready(ITaskControlBlock *task) override;

    /**
     * 任务注册：将创建好的控制块纳入管理器追踪范围
     */
    void register_task(ITaskControlBlock *tcb);

    /**
     * 获取任务实例：返回通用控制接口
     */
    ITaskControlBlock *get_task(uint32_t task_id);

private:
    ITaskControlBlock *create_and_register(TaskEntry entry, void *arg, TaskPriority prio);
};