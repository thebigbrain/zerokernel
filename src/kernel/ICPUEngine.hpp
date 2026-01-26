#pragma once

#include "ITaskContext.hpp"
/**
 * CPU 执行器接口：内核眼中 CPU 的样子
 */

class ICPUEngine
{
public:
    // 核心能力：接受一个上下文，并开始运行它
    virtual void execute(ITaskContext *context) = 0;

    // 状态切换：从当前上下文切换到下一个
    virtual void transit(ITaskContext *current, ITaskContext *next) = 0;

    // 控制能力
    virtual void halt() = 0;                        // 停机
    virtual void interrupt_enable(bool enable) = 0; // 中断控制
};