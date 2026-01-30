#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct WinX64Regs
{
    // 这些成员对应汇编中的 pop 顺序（从小到大地址）
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbx;
    uint64_t rbp;
    uint64_t rip; // 对应 ret 指令从栈顶弹出的返回地址
};
#pragma pack(pop)