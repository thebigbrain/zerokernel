#pragma once
#include <kernel/ITaskContext.hpp>
#include <cstdint>
#include "WinX64Regs.hpp"

class WinTaskContext : public ITaskContext
{
private:
    // sp 指向栈上保存 WinX64Regs 结构体的位置
    WinX64Regs *sp = nullptr;

    // x64 寄存器传参通常前 4 个使用寄存器，其余入栈
    // 这里我们先支持 4 个寄存器参数
    uintptr_t m_args[4] = {0, 0, 0, 0};

    void *entry_func = nullptr;
    void *stack_top = nullptr;

    void *_exit_stub = nullptr;
    uint32_t _shadow_space_size = 32;

    void update_regs_from_args();

public:
    WinTaskContext() : WinTaskContext(nullptr, 32) {}
    // 构造函数
    WinTaskContext(void *exit_stub, uint32_t shadow_space_size = 32);

    size_t get_context_size() const;

    /**
     * 核心动作：从当前执行流切换到另一个执行流
     * @param target 目标上下文
     * 内部实现：保存当前寄存器到 this，从 target 恢复寄存器并跳转
     */
    void transit_to(ITaskContext *target);

    void load_argument(size_t index, uintptr_t value) override;

    // 移除多余的类名前缀
    void setup_flow(void (*entry)(void *, void *), void *stack_top) override;

    void *get_stack_pointer() const override { return sp; }

private:
    void setup_registers();
};