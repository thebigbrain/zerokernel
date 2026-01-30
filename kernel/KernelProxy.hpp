#pragma once

#include "common/IUserRuntime.hpp"
#include "MessageBus.hpp"
#include "Kernel.hpp"

class KernelProxy : public IUserRuntime
{
private:
    MessageBus *_bus;
    Kernel *_kernel;

public:
    KernelProxy(MessageBus *bus, Kernel *k) : _bus(bus), _kernel(k) {}

    // 纯粹的消息透传
    void publish(const Message &msg) override
    {
        _bus->publish(msg);
    }

    // 纯粹的控制权转交
    void yield() override
    {
        _kernel->yield();
    }
};
