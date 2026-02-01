#pragma once
#include <kernel/ICPUEngine.hpp>
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

    void update_regs_from_args();

public:
    // 构造函数
    WinTaskContext() = default;

    void set_parameter(int index, uintptr_t value) override;

    // 移除多余的类名前缀
    void prepare(void (*entry)(), void *stack_top, void (*exit_router)()) override;

    void *get_stack_pointer() const override { return sp; }
    void set_stack_pointer(void *s) override { sp = (WinX64Regs *)s; }
};