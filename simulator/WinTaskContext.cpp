#include "WinTaskContext.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <kernel/ISchedulingControl.hpp>

extern "C" void context_switch_asm(void **old_sp, void *new_sp);
// extern "C" void context_load_asm(void *sp);

void platform_task_exit_stub()
{
    // 假设可以通过某种方式获取到当前的控制接口
    // 或者直接触发底层 Trap 信号
    extern ISchedulingControl *g_platform_sched_ctrl;
    g_platform_sched_ctrl->terminate_current_task();
}

WinTaskContext::WinTaskContext(void *exit_stub, uint32_t shadow_space_size)
{
    if (exit_stub)
    {
        this->_exit_stub = exit_stub;
    }
    else
    {
        this->_exit_stub = platform_task_exit_stub;
    }

    this->_shadow_space_size = shadow_space_size;
}

void WinTaskContext::transit_to(ITaskContext *target)
{
    // 强制转换为具体实现类以获取其 sp
    auto *next_ctx = static_cast<WinTaskContext *>(target);

    // 调用汇编：
    // 第一个参数 (RCX): 当前 sp 成员变量的地址 (&this->sp)
    // 第二个参数 (RDX): 目标 sp 的值 (next_ctx->sp)
    context_switch_asm(reinterpret_cast<void **>(&this->sp), next_ctx->sp);
}

size_t WinTaskContext::get_context_size() const
{
    return sizeof(WinX64Regs);
}

void WinTaskContext::setup_flow(void (*entry)(void *, void *), void *stack_top)
{
    this->entry_func = entry;
    this->stack_top = stack_top;

    this->setup_registers();

    // 6. 将暂存的所有参数一次性写入寄存器镜像
    update_regs_from_args();
}

void WinTaskContext::setup_registers()
{
    uintptr_t curr = reinterpret_cast<uintptr_t>(this->stack_top);
    curr &= ~0xFULL; // 强制 16 字节对齐 (16n)

    // 1. 影子空间 (Shadow Space)
    // 根据 ABI，它是调用者为被调用者准备的“救生圈”
    curr -= 32;

    // 2. 退出桩 (Exit Stub)
    // 它是 main 函数返回后的“落脚点”，位于影子空间的正下方
    curr -= 8;
    *reinterpret_cast<uintptr_t *>(curr) = reinterpret_cast<uintptr_t>(_exit_stub);

    // 此时的 curr 必须满足 curr % 16 == 8，因为 ret 弹出后 RSP 变 16n
    // 这种布局下，进入 main 后的第一条指令，RSP 将对齐在 16n + 8，完美！

    // 3. 任务入口点 (RIP)
    // 给 context_switch_asm 最后的 ret 使用
    curr -= 8;
    *reinterpret_cast<uintptr_t *>(curr) = reinterpret_cast<uintptr_t>(this->entry_func);

    // 4. 寄存器镜像区
    curr -= sizeof(WinX64Regs);
    this->sp = reinterpret_cast<WinX64Regs *>(curr);

    memset(this->sp, 0, sizeof(WinX64Regs));

    // 5. 刷入参数
    update_regs_from_args();
}

void WinTaskContext::load_argument(size_t index, uintptr_t value)
{
    if (index < 0 || index >= 4)
        return; // 防止越界

    // 1. 暂存在数组中
    m_args[index] = value;

    // 2. 如果 sp 已经 setup_flow 好了，直接同步到内存镜像
    if (this->sp)
    {
        update_regs_from_args();
    }
}

// 辅助函数：将数组值刷入寄存器镜像
void WinTaskContext::update_regs_from_args()
{
    if (!this->sp)
        return;

    // 映射关系：0->RCX, 1->RDX, 2->R8, 3->R9
    this->sp->rcx = m_args[0];
    this->sp->rdx = m_args[1];
    this->sp->r8 = m_args[2];
    this->sp->r9 = m_args[3];
}