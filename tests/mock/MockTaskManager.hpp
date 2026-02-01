#pragma once

#include "kernel/ITaskManager.hpp"
#include <cstdio>

class MockTaskManager : public ITaskManager
{
public:
    bool exit_called = false;

    void terminate_current_task() override
    {
        printf("[Mock TM] Task exit sequence captured.\n");
        exit_called = true;
        // 在同步测试模式下，我们可以直接 throw 一个异常
        // 这样测试框架就能捕获到任务运行结束了
        throw TaskFinishedException();
    }

    void yield_current_task() override
    {
        // 同步测试下，yield 可以直接空转或打印日志
        printf("[Mock TM] Yielding logic skipped in sync test.\n");
    }

    // ... 实现其他接口 ...
};