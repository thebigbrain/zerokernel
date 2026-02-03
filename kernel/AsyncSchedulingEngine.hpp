#pragma once

#include "IExecutionEngine.hpp"
#include "ITaskLifecycle.hpp"
#include "ISchedulingControl.hpp"
#include "ISchedulingStrategy.hpp"

class AsyncSchedulingEngine : public IExecutionEngine
{
private:
    ITaskLifecycle *_lifecycle;         // 查当前任务
    ISchedulingControl *_sched_control; // 执行 yield
    ISchedulingStrategy *_strategy;     // 选下一个任务

public:
    AsyncSchedulingEngine(ITaskLifecycle *lc, ISchedulingControl *sc, ISchedulingStrategy *st)
        : _lifecycle(lc), _sched_control(sc), _strategy(st) {}

    void start() override
    {
        ITaskControlBlock *first = _strategy->pick_next_ready_task();
        if (first)
        {
            _lifecycle->register_task(first);
            first->get_context()->jump_to(); // 开启万物
        }
    }

    void schedule_task(ITaskControlBlock *tcb) override
    {
        auto current_task = _lifecycle->get_current_task();
        auto priority = tcb->get_resource_config().priority;
        // 这里体现策略：如果是抢占式，就在这里对比优先级
        if (priority > current_task->get_resource_config().priority)
        {
            _sched_control->yield_current_task(); // 触发切换
        }
    }
};