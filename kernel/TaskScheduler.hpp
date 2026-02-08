#pragma once

#include "ITaskControlBlock.hpp"
#include "ISchedulingStrategy.hpp"
#include "ISchedulingPolicy.hpp"

class TaskScheduler
{
public:
    TaskScheduler(ISchedulingStrategy *strategy, ISchedulingPolicy *policy)
        : _strategy(strategy), _policy(policy) {}

    void yield_current()
    {
        ITaskControlBlock *current = _current_running;
        ITaskControlBlock *next = _strategy->pick_next_ready_task();

        if (!next || next == current)
        {
            // 没有更合适的任务，继续跑当前任务
            return;
        }

        // 1. 状态维护：旧任务归队（Strategy 决定放哪）
        _strategy->make_task_ready(current);

        // 2. 状态切换：更新当前指针
        _current_running = next;

        // 3. 物理执行：触发上下文切换
        // 注意：这是跨越时空的瞬间
        current->get_context()->transit_to(next->get_context());
    }

    void switch_to(ITaskControlBlock *next)
    {
        if (!next)
            return;

        ITaskControlBlock *prev = _current_running;

        // 1. 语义检查：如果切向自己，直接返回
        if (prev == next)
            return;

        // 2. 更新逻辑状态：谁在跑？
        // 必须在物理切换前更新，因为一旦进入 transit_to，当前函数的执行流就会暂停
        _current_running = next;

        K_DEBUG("Scheduler: Context Switch [%s] -> [%s]",
                prev ? prev->get_name() : "NONE",
                next->get_name());

        // 3. 执行物理切换
        // 如果 prev 为空（比如系统刚启动时），我们需要特殊的处理，或者确保 prev 始终有效
        if (prev)
        {
            prev->get_context()->transit_to(next->get_context());
        }
        else
        {
            // 处理初始切换：如果是从零启动，建议还是用你之前的 idle_tcb 方案
            // 或者直接调用汇编来加载 next 的状态
            K_PANIC("Scheduler: Initial Context Switch, No Previous Task");
        }

        // --- 临界区边界 ---
        // 当代码运行到这一行时，说明 CPU 已经重新切换回了 prev 任务
        // 此时必须把“当前任务”重新设为 prev，否则 prev 后续的逻辑会认为自己在跑别人
        _current_running = prev;
    }

    void set_current(ITaskControlBlock *tcb) { _current_running = tcb; }
    ITaskControlBlock *get_current() { return _current_running; }

private:
    ITaskControlBlock *_current_running = nullptr;
    ISchedulingStrategy *_strategy;
    ISchedulingPolicy *_policy;
};