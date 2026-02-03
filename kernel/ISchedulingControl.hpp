#pragma once

// 只负责切换和让出执行权的最小接口
struct ISchedulingControl
{
    virtual void yield_current_task() = 0;
    virtual void terminate_current_task() = 0;
    virtual ~ISchedulingControl() = default;
};