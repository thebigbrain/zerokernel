#pragma once

#include "ITaskContext.hpp"
/**
 * CPU 执行器接口：内核眼中 CPU 的样子
 */

class ICPUEngine
{
public:
    // 让内核知道需要分配多少内存
    virtual size_t get_context_size() const = 0;

    // 在指定地址构造一个架构相关的上下文对象
    virtual ITaskContext *create_context_at(void *address) = 0;

    // 核心能力：接受一个上下文，并开始运行它
    virtual void switch_to(ITaskContext *context) = 0;

    // 状态切换：从当前上下文切换到下一个
    virtual void transit(ITaskContext *current, ITaskContext *next) = 0;

    // 控制能力
    virtual void halt() = 0;                        // 停机
    virtual void interrupt_enable(bool enable) = 0; // 中断控制
};