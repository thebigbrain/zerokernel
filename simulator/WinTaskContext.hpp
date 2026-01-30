#pragma once
#include <kernel/ICPUEngine.hpp>
#include <windows.h>
#include <cstdint>
#include "WinX64Regs.hpp"

class WinTaskContext : public ITaskContext
{
public:
    void *sp = nullptr;

    // 修复：set_parameter 必须操作“栈上的寄存器镜像”
    void set_parameter(int index, uintptr_t value) override
    {
        if (!sp)
            return; // 必须先调用 prepare 分配栈空间

        // 强转回结构体指针，直接修改栈里的初始值
        auto *regs = (WinX64Regs *)sp;

        // 根据你选择的 ABI (System V 常用 RDI/RSI，Windows 常用 RCX/RDX)
        switch (index)
        {
        case 0:
            regs->rdi = value; // 对应 root_task_entry 的第一个参数 rt
            break;
        case 1:
            regs->rsi = value; // 对应第二个参数
            break;
        }
    }

    void prepare(void (*entry)(), void *stack_top) override
    {
        // 1. 在栈顶预留寄存器结构体的空间
        // 注意：x64 栈需要 16 字节对齐，这里减去结构体大小
        auto *regs = (WinX64Regs *)((uintptr_t)stack_top - sizeof(WinX64Regs));

        // 2. 清零初始寄存器状态，防止随机值干扰
        memset(regs, 0, sizeof(WinX64Regs));

        // 3. 设置初始指令指针
        regs->rip = (uintptr_t)entry;

        // 4. 设置初始基址指针（可选）
        regs->rbp = 0;

        // 5. 重要：更新 sp，使其指向这个结构体的起始位置
        // 这样 context_switch 汇编代码就能通过这个地址开始 pop 寄存器
        this->sp = regs;
    }

    void *get_stack_pointer() const override { return sp; }
    void set_stack_pointer(void *s) override { sp = s; }
};