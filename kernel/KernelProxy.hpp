#pragma once

#include "common/IUserRuntime.hpp"
#include "MessageBus.hpp"
#include "ITaskManager.hpp"

class KernelProxy : public IUserRuntime
{
private:
    MessageBus *_bus;
    ITaskManager *_task_manager; // 核心调整：改为依赖任务管理器接口

public:
    // 构造函数注入：这使得测试时可以注入 MockBus 和 MockTaskManager
    KernelProxy(MessageBus *bus, ITaskManager *tm)
        : _bus(bus), _task_manager(tm) {}

    // 消息投递：依然是透传给总线
    void publish(const Message &msg) override
    {
        _bus->publish(msg);
    }

    // 协作调度：转交给任务管理器
    void yield() override
    {
        // 领域语义：任务请求让出执行权，管理器决定切给谁
        _task_manager->yield_current_task();
    }
};