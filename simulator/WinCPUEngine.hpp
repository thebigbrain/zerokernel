#pragma once

#include <kernel/ICPUEngine.hpp>
#include <windows.h>
#include <cstdint>
#include "WinX64Regs.hpp"

class WinCPUEngine : public ICPUEngine
{
public:
    size_t WinCPUEngine::get_context_size() const override;

    ITaskContext *WinCPUEngine::create_context_at(void *address);

    void execute(ITaskContext *context) override;

    void transit(ITaskContext *current, ITaskContext *next) override;

    void halt() override { ExitProcess(0); }
    void interrupt_enable(bool enable) override { /* 模拟层暂不实现 */ }
};