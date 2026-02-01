#pragma once

#include <queue>
#include "ICPUEngine.hpp"
#include "task.hpp"
#include "ObjectFactory.hpp"
#include "ITaskManager.hpp"
#include <common/Message.hpp>
#include "MessageBus.hpp"
#include <common/IUserRuntime.hpp>
#include <common/BootInfo.hpp>

class Kernel : public ITaskManager
{
private:
    BootInfo *_boot_info;

    ICPUEngine *_cpu;
    Task *_current;
    Task *_next;

    Task *_idle_task;

    ObjectFactory *_factory;
    // 假设我们有一个任务链表或数组来管理所有 Task
    Task *_tasks[32];
    int _task_count = 0;
    int _current_index = 0;

    std::queue<Task *> _ready_queue;

    MessageBus *_bus;

    static Kernel *instance;

    Task *create_task_skeleton(void (*entry_point)());

public:
    Kernel(ICPUEngine *cpu, ObjectFactory *factory) : _cpu(cpu), _factory(factory), _current(nullptr), _next(nullptr)
    {
        if (Kernel::instance)
        {
            return;
        }
        Kernel::instance = this;
    }

    static Kernel *get_instance()
    {
        return instance;
    }

    void bootstrap(BootInfo *info);

    void handle_load_task(const Message &msg);
    void handle_event_print(const Message &msg);

    void run_loop();

    // 补全方法：获取指定索引的任务
    Task *get_ready_task(int index);

    // 协作式调度示例
    void yield();

    // 动态创建任务并加入队列
    Task *spawn_task(void (*entry_point)());
    Task *spawn_fixed_task(void *task_entry, void *config);

    void terminate_current_task() override
    {
        // 1. 标记当前任务为销毁状态
        // 2. 触发 yield() 切换到下一个任务
        // 3. 这里的逻辑完全不涉及寄存器
        this->yield();
    }
};
