#pragma once

#include "ITaskContext.hpp"

class ITaskContextFactory
{
public:
    virtual ~ITaskContextFactory() = default;

    /**
     * 核心方法：产出一个全新的、初始化的上下文
     * 内部负责内存申请，或者调用 ObjFactory 申请
     */
    virtual ITaskContext *create_context() = 0;

    /**
     * 回收上下文
     */
    virtual void destroy_context(ITaskContext *ctx) = 0;
};