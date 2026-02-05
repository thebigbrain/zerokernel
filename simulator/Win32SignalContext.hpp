#pragma once

#include <windows.h>
#include <cstdint>

#include "kernel/ISignal.hpp"

class Win32SignalContext : public ISignalContext
{
private:
    // 存储 Windows 获取到的原始线程上下文快照
    CONTEXT _context;

public:
    explicit Win32SignalContext(const CONTEXT &ctx) : _context(ctx) {}

    /**
     * @brief 获取指令指针 (PC/IP)
     */
    uintptr_t get_instruction_pointer() const override
    {
#if defined(_M_X64) || defined(__x86_64__)
        return static_cast<uintptr_t>(_context.Rip);
#else
        return static_cast<uintptr_t>(_context.Eip);
#endif
    }

    /**
     * @brief 获取栈指针 (SP)
     */
    uintptr_t get_stack_pointer() const override
    {
#if defined(_M_X64) || defined(__x86_64__)
        return static_cast<uintptr_t>(_context.Rsp);
#else
        return static_cast<uintptr_t>(_context.Esp);
#endif
    }

    /**
     * @brief 设置通用返回值寄存器 (通常用于系统调用)
     * 在 x86/x64 中，返回值通常存储在 EAX/RAX
     */
    void set_return_value(uintptr_t value) override
    {
#if defined(_M_X64) || defined(__x86_64__)
        _context.Rax = static_cast<DWORD64>(value);
#else
        _context.Eax = static_cast<DWORD>(value);
#endif
    }

    /**
     * @brief 获取原始 Windows CONTEXT
     * 允许分发器最后通过 SetThreadContext 将修改后的现场写回线程
     */
    const CONTEXT &get_raw_context() const { return _context; }
};