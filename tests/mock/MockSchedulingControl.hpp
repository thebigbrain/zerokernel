#pragma once

#include <iostream>

#include "kernel/ISchedulingControl.hpp"

class MockSchedulingControl : public ISchedulingControl
{
public:
    bool yield_called = false;

    void yield_current_task() override
    {
        yield_called = true;
        std::cout << "[Mock] Task requested yield." << std::endl;
    }

    void terminate_current_task() override
    {
        std::cout << "[Mock] Task requested termination." << std::endl;
        // 实际模拟器中这里会切换回内核栈
    }
};