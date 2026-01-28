#include "WinCPUEngine.hpp"
#include <new>
#include <iostream>

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
    // 获取具体实现的成员变量 sp 的地址
    // 强制转换为 WinTaskContext 是安全的，因为这个函数位于 arch 模块内
    WinTaskContext *curWin = static_cast<WinTaskContext *>(current);
    WinTaskContext *nxtWin = static_cast<WinTaskContext *>(next);

    context_switch_asm(&curWin->sp, nxtWin->sp);
}

void WinCPUEngine::execute(ITaskContext *context)
{
    void *dummy;
    context_switch_asm(&dummy, static_cast<WinTaskContext *>(context)->sp);
}