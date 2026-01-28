#pragma once

#include <kernel/ICPUEngine.hpp>
#include <windows.h>
#include <cstdint>

// 与汇编约定的布局相同
struct WinX64Regs
{
    uint64_t r15, r14, r13, r12, rsi, rdi, rbx, rbp;
    uint64_t rip;
};

class WinTaskContext : public ITaskContext
{
public:
    void *sp = nullptr;

    void prepare(void (*entry)(), void *stack_top) override
    {
        auto *regs = (WinX64Regs *)((uintptr_t)stack_top - sizeof(WinX64Regs));
        regs->rip = (uintptr_t)entry;
        regs->rbp = 0;
        this->sp = regs;
    }

    void *get_stack_pointer() const override { return sp; }
    void set_stack_pointer(void *s) override { sp = s; }
};

// 汇编原语声明
extern "C" void context_switch_asm(void **old_sp, void *new_sp);

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