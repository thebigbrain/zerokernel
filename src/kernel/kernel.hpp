#pragma once

#include "ICPUEngine.hpp"
#include "Task.hpp"
#include "ObjectFactory.hpp"
#include "ITaskManager.hpp"

class Kernel : public ITaskManager
{
private:
    ICPUEngine *_cpu;
    Task *_current;
    Task *_next;

    ObjectFactory *_factory;
    // 假设我们有一个任务链表或数组来管理所有 Task
    Task *_tasks[32];
    int _task_count = 0;
    int _current_index = 0;

public:
    Kernel(ICPUEngine *cpu, ObjectFactory *factory) : _cpu(cpu), _factory(factory), _current(nullptr), _next(nullptr)
    {
    }

    void Kernel::bootstrap(void (*idle_logic)());

    // 补全方法：获取指定索引的任务
    Task *get_ready_task(int index)
    {
        if (index < _task_count)
        {
            return _tasks[index];
        }
        return nullptr; // 或者返回 Idle 任务
    }

    // 协作式调度示例
    void yield()
    {
        if (_task_count < 2)
            return;

        int old_index = _current_index;
        _current_index = (_current_index + 1) % _task_count;

        Task *prev = _tasks[old_index];
        Task *next = _tasks[_current_index];

        _cpu->transit(prev->get_context(), next->get_context());
    }

    // 动态创建任务并加入队列
    Task *spawn_task(void (*entry_point)())
    {
        if (_task_count >= 64)
            return nullptr;

        // 1. 架构无关地创建 Context
        size_t ctx_size = _cpu->get_context_size();
        void *ctx_mem = _factory->allocate_raw(ctx_size);
        ITaskContext *ctx = _cpu->create_context_at(ctx_mem);

        // 2. 创建 Task 对象
        Task *newTask = _factory->create<Task>(_task_count, ctx);

        // 3. 分配栈空间
        size_t stack_size = 16384;
        void *stack_bottom = _factory->allocate_raw(stack_size); // 拿到的是低地址
        void *stack_top = (uint8_t *)stack_bottom + stack_size;  // 计算高地址（栈顶）

        // 4. 初始化（在之前的讨论中，这里会填入 entry 和 exit_gate）
        newTask->prepare(entry_point, stack_top);

        // 5. 注册到就绪数组
        _tasks[_task_count++] = newTask;

        return newTask;
    }

    void terminate_current_task() override
    {
        // 1. 标记当前任务为销毁状态
        // 2. 触发 yield() 切换到下一个任务
        // 3. 这里的逻辑完全不涉及寄存器
        this->yield();
    }
};
