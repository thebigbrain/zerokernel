#pragma once

#include "ICPUEngine.hpp"
#include "Task.hpp"

class Kernel
{
private:
    ICPUEngine *_cpu;
    Task *_current;
    Task *_next;

public:
    Kernel(ICPUEngine *cpu) : _cpu(cpu), _current(nullptr), _next(nullptr) {}

    void set_tasks(Task *t1, Task *t2)
    {
        _current = t1;
        _next = t2;
    }

    // 协作式调度示例
    void yield()
    {
        Task *prev = _current;
        _current = _next;
        _next = prev;

        // 内核下达执行指令，完全不触碰寄存器
        _cpu->transit(prev->get_context(), _current->get_context());
    }
};