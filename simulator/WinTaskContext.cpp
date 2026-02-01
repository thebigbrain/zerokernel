#include "WinTaskContext.hpp"
#include <cstring>

void WinTaskContext::prepare(void (*entry)(), void *stack_top, void (*exit_router)())
{
    // 1. 初始对齐 (16n)
    uintptr_t curr = reinterpret_cast<uintptr_t>(stack_top) & ~0xFULL;

    // 2. 压入 Exit Router 和 影子空间
    curr -= 8;
    *reinterpret_cast<uintptr_t *>(curr) = reinterpret_cast<uintptr_t>(exit_router);
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

void WinTaskContext::set_parameter(int index, uintptr_t value)
{
    if (index < 0 || index >= 4)
        return; // 防止越界

    // 1. 暂存在数组中
    m_args[index] = value;

    // 2. 如果 sp 已经 prepare 好了，直接同步到内存镜像
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