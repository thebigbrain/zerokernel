#pragma once

#include "ITaskContext.hpp"
/**
 * CPU 执行器接口：内核眼中 CPU 的样子
 */

class ICPUEngine
{
public:
    // 控制能力
    virtual void halt() = 0;                        // 停机
    virtual void interrupt_enable(bool enable) = 0; // 中断控制
};