#pragma once

#include "kernel/ICPUEngine.hpp"
#include "kernel/SimpleTaskLifecycle.hpp"

class SyncTestEngine : public ICPUEngine
{
private:
    SimpleTaskLifecycle *_lifecycle;

public:
    SyncTestEngine(SimpleTaskLifecycle *lifecycle) : _lifecycle(lifecycle) {}

    void halt() override {}
    void interrupt_enable(bool) override {}

    void start()
    {
        auto *current = _lifecycle->get_current_task();
        if (current)
        {
            // 重点：直接利用接口的多态性
            // 如果是 MockTaskContext，它会直接运行函数
            // 如果是 WinTaskContext，它会执行真实的跳转（但在同步测试中我们用 Mock）
            current->get_context()->jump_to();
        }
    }
};