#pragma once
#include "ISchedulingControl.hpp"
#include "ITaskLifecycle.hpp"

extern "C" void set_active_scheduler(ISchedulingControl *mgr);

class SimpleSchedulingControl : public ISchedulingControl
{
private:
    ITaskLifecycle *_lifecycle; // 需要知道当前是谁在跑

public:
    SimpleSchedulingControl(ITaskLifecycle *lc) : _lifecycle(lc)
    {
        set_active_scheduler(this);
    }

    void yield_current_task() override
    {
        // 逻辑：触发软件中断（如 ARM 的 PendSV 或 x86 的 INT）
        // 或者直接调用 ExecutionEngine 强制重新调度
        // 这是一个物理操作
    }

    void terminate_current_task() override
    {
        ITaskControlBlock *current = _lifecycle->get_current_task();
        if (current)
        {
            // 1. 标记状态为 DEAD
            // 2. 触发 yield 切换到下一个
            yield_current_task();
        }
    }
};