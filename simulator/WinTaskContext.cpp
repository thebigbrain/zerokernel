#include "WinTaskContext.hpp"
#include <cstring>
#include <iostream>
#include <kernel/ISchedulingControl.hpp>

extern "C" void context_switch_asm(void **old_sp, void *new_sp);
extern "C" void context_load_asm(void *sp);

void platform_task_exit_stub()
{
    // 假设可以通过某种方式获取到当前的控制接口
    // 或者直接触发底层 Trap 信号
    extern ISchedulingControl *g_platform_sched_ctrl;
    g_platform_sched_ctrl->terminate_current_task();
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

void WinTaskContext::jump_to()
{
    std::cout << "WinTaskContext::jump_to()" << sp << std::endl;
    // 直接丢弃当前上下文，加载 sp
    context_load_asm(sp);
    std::cout << "Target RIP: " << (void *)sp->rip << std::endl;
}

size_t WinTaskContext::get_context_size() const
{
    return sizeof(WinX64Regs);
}

void WinTaskContext::setup_flow(void (*entry)(), void *stack_top)
{
    // 1. 初始对齐 (16n)
    uintptr_t curr = reinterpret_cast<uintptr_t>(stack_top) & ~0xFULL;

    // 2. 压入 Exit Router 和 影子空间
    curr -= 8;
    *reinterpret_cast<uintptr_t *>(curr) = reinterpret_cast<uintptr_t>(platform_task_exit_stub);
    curr -= 32;

    // 3. 放置寄存器上下文 (88字节)
    curr -= sizeof(WinX64Regs);

    // 4. 初始化结构体
    this->sp = reinterpret_cast<WinX64Regs *>(curr);
    memset(this->sp, 0, sizeof(WinX64Regs));

    // 5. 写入关键寄存器
    this->sp->rip = reinterpret_cast<uintptr_t>(entry);

    // 6. 将暂存的所有参数一次性写入寄存器镜像
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