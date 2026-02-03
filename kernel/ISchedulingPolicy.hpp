#pragma once

#include "ITaskControlBlock.hpp"

struct ISchedulingPolicy
{
    // 决定任务在当前状态下的优先级（动态优先级）
    virtual TaskPriority calculate_priority(ITaskControlBlock *tcb) = 0;

    // 决定给这个任务分配多少执行时间（时间片长度）
    virtual uint32_t get_time_slice_ms(ITaskControlBlock *tcb) = 0;

    // 决定是否应该立即发生抢占
    // 例如：当新任务 A 变成 Ready 时，是否要踢掉正在运行的 B？
    virtual bool should_preempt(ITaskControlBlock *current, ITaskControlBlock *next) = 0;
};