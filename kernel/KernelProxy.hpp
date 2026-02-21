#pragma once

#include "common/diagnostics.hpp"
#include "common/IUserRuntime.hpp"
#include "IMessageBus.hpp"
#include "ISchedulingControl.hpp"
#include "ResourceManager.hpp"

class KernelRuntimeProxy : public IUserRuntime
{
private:
    IMessageBus *_bus;
    ISchedulingControl *_sched; // 核心调整：改为依赖任务管理器接口
    PlatformHooks *_hooks;

public:
    // 构造函数注入：这使得测试时可以注入 MockBus 和 MockTaskManager
    KernelRuntimeProxy(IMessageBus *bus, PlatformHooks *hooks)
        : _bus(bus), _hooks(hooks) {}

    // 消息投递：依然是透传给总线
    void publish(const Message &msg) override
    {
        if (msg.type == MessageType::REQUEST_HARDWARE_INFO)
        {
            const char *hw_name = (const char *)msg.payload[0];
            uintptr_t *out_ptr = (uintptr_t *)msg.payload[1];

            auto *res = _hooks->resource_manager->query(hw_name);
            if (res)
            {
                *out_ptr = res->base_address;
                // klog(LogLevel::Info, "Kernel: Assigned %s at 0x%p", hw_name, res->base_address);
            }
            else
            {
                *out_ptr = 0;
            }
        }
        else if (msg.type == MessageType::EVENT_VRAM_UPDATED)
        {
            _hooks->refresh_display();
        }
    }

    // 协作调度：转交给任务管理器
    void yield() override
    {
        if (!_sched)
            return;
        // 领域语义：任务请求让出执行权，管理器决定切给谁
        _hooks->sched_control->yield_current_task();
    }
};