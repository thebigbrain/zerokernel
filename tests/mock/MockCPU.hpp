#pragma once

#include "kernel/ICPUEngine.hpp"

class MockCPU : public ICPUEngine
{
public:
    void halt() override {}

    void interrupt_enable(bool enable) {}
};