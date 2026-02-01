#include "WinCPUEngine.hpp"
#include <new>
#include <iostream>
#include "WinTaskContext.hpp"
#include "asmdefs.hpp"

// 返回 Windows 模拟环境下的上下文结构大小
size_t WinCPUEngine::get_context_size() const
{
    return sizeof(WinTaskContext);
}

ITaskContext *WinCPUEngine::create_context_at(void *address)
{
    if (!address)
    {
        std::cout << "Memory allocation failed" << std::endl;
        return nullptr; // 内存分配失败保护
    }

    // 使用 placement new 在内核分配的内存上构造具体实现类
    return new (address) WinTaskContext();
}

void WinCPUEngine::transit(ITaskContext *current, ITaskContext *next)
{
    // 目标上下文必须存在
    if (next == nullptr)
        return;

    WinTaskContext *nxtWin = static_cast<WinTaskContext *>(next);

    if (current == nullptr)
    {
        // --- 场景：第一次启动任务 ---
        // 我们不需要保存任何当前寄存器，只需将 CPU 的 SP 切换到目标任务，
        // 然后执行弹栈（恢复现场）逻辑。
        auto sp = nxtWin->get_stack_pointer();
        context_load_asm(sp);
    }
    else
    {
        // --- 场景：正常的任务切换 ---
        WinTaskContext *curWin = static_cast<WinTaskContext *>(current);

        // 保存当前 SP 到 curWin->sp，并切换到 nxtWin->sp
        auto curSp = curWin->get_stack_pointer();
        context_switch_asm(&curSp, nxtWin->get_stack_pointer());
    }
}

void WinCPUEngine::execute(ITaskContext *context)
{
    transit(nullptr, context);
}