#pragma once

#include "kernel/ITaskContextFactory.hpp"
#include "WinTaskContext.hpp"

// 位于模拟器工程，直接依赖具体的 WinTaskContext
class WinTaskContextFactory : public ITaskContextFactory
{
public:
    WinTaskContextFactory() = default;

    ITaskContext *create_context() override
    {
        // 直接在模拟器堆上创建具体对象
        return new WinTaskContext();
    }

    void destroy_context(ITaskContext *ctx) override
    {
        // 既然是这里 new 出来的，就必须在这里 delete
        // 注意：由于 ITaskContext 析构函数是 virtual，这里会正确调用 WinTaskContext 的析构
        delete ctx;
    }
};