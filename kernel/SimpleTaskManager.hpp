#pragma once
#include "ITaskManager.hpp"
#include "ObjectFactory.hpp"
#include "ICPUEngine.hpp"
#include "ITaskControlBlock.hpp"
#include "TaskControlBlock.hpp" // 必须包含具体实现
#include <queue>                // 补全 queue

class SimpleTaskManager : public ITaskManager
{
private:
    ObjectFactory *_factory;
    ICPUEngine *_cpu;
    ITaskControlBlock *_current = nullptr;
    std::queue<ITaskControlBlock *> _ready_queue;
    ITaskControlBlock *_tasks[64];
    uint32_t _task_count = 0;

    BootInfo *_boot_info;

public:
    SimpleTaskManager(ObjectFactory *f, ICPUEngine *cpu, BootInfo *info);
    ~SimpleTaskManager() override;

    // 接口实现
    ITaskControlBlock *spawn_task(void *entry_point) override; // 修正返回指针
    void spawn_fixed_task(void *entry, void *config, void *proxy) override;
    void spawn_task_from_message(const Message &msg) override;

    BootInfo *get_boot_info() override;
    void terminate_current_task() override;
    void yield_current_task() override;

    ITaskControlBlock *pick_next_ready_task() override;
    void make_task_ready(ITaskControlBlock *task) override;

    // 占位实现（如有需要可后续补完）
    void register_task(uint32_t task_id, void *task_ptr) override {}
    void *get_task(uint32_t task_id) override { return (task_id < 64) ? _tasks[task_id] : nullptr; }

private:
    ITaskControlBlock *create_tcb_internal(void (*entry)(), uint32_t id);
};