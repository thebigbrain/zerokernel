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
    void execute(ITaskContext *context) override
    {
        // 初始启动：直接强行加载栈指针并跳转
        void *dummy_old_sp;
        context_switch_asm(&dummy_old_sp, context->get_stack_pointer());
    }

    void transit(ITaskContext *current, ITaskContext *next) override
    {
        // 调用汇编原语
        void **old_sp_ptr = (void **)current; // 这里的 hack 取决于 WinTaskContext 的布局
        // 更稳妥的做法：
        void *next_sp = next->get_stack_pointer();

        // 获取 current 对象中存储 sp 的成员地址
        // 假设 WinTaskContext 的第一个成员就是 sp
        context_switch_asm(&(static_cast<WinTaskContext *>(current)->sp), next_sp);
    }

    void halt() override { ExitProcess(0); }
    void interrupt_enable(bool enable) override { /* 模拟层暂不实现 */ }
};