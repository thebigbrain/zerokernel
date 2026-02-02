#pragma once

#include <kernel/ICPUEngine.hpp>
#include <windows.h>
#include <cstdint>
#include "WinX64Regs.hpp"

class WinCPUEngine : public ICPUEngine
{
public:
    void halt() override { ExitProcess(0); }
    void interrupt_enable(bool enable) override { /* 模拟层暂不实现 */ }
};