#pragma once

#include "kernel/IExecutionEngine.hpp"
#include "common/IUserRuntime.hpp"

class SyncTestEngine : public IExecutionEngine
{
public:
    void start(void *entry, void *arg, IUserRuntime *rt) override
    {
        // 1. 同步模式下，我不需要管理器去 spawn 任务，不需要分配栈
        // 2. 我也不需要 run_loop
        // 3. 我直接以 C++ 方式调用入口
        auto task_func = (void (*)(IUserRuntime *, void *))entry;
        task_func(rt, arg);

        // 执行完直接返回，测试通过！
    }
};