#pragma once

#include "common/diagnostics.hpp"
#include "common/IUserRuntime.hpp"
#include "IMessageBus.hpp"
#include "ISchedulingControl.hpp"

class KernelProxy : public IUserRuntime
{
private:
    IMessageBus *_bus;
    ISchedulingControl *_sched; // 核心调整：改为依赖任务管理器接口

public:
    // 构造函数注入：这使得测试时可以注入 MockBus 和 MockTaskManager
    KernelProxy(IMessageBus *bus, ISchedulingControl *sc)
        : _bus(bus), _sched(sc) {}

    // 消息投递：依然是透传给总线
    void publish(const Message &msg) override
    {
        if (!_bus)
            return;
        _bus->publish(msg);
    }

    // 协作调度：转交给任务管理器
    void yield() override
    {
        if (!_sched)
            return;
        // 领域语义：任务请求让出执行权，管理器决定切给谁
        _sched->yield_current_task();
    }
};