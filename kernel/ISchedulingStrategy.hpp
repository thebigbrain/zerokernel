#pragma once
#include "ITaskControlBlock.hpp"

struct ISchedulingStrategy
{
    // 决策：谁是下一个？
    virtual ITaskControlBlock *pick_next_ready_task() = 0;

    // 更新：这个任务现在可以跑了，请归队
    virtual void make_task_ready(ITaskControlBlock *tcb) = 0;

    // 可选：任务退出时通知策略移除它
    virtual void remove_task(ITaskControlBlock *tcb) = 0;
};