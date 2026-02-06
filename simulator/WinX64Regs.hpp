#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct WinX64Regs
{
    // --- 易失性寄存器（用于首航参数传递） ---
    // 偏移 0x00: 物理地址最低，RSP 指向这里
    uint64_t rcx;
    uint64_t rdx;
    uint64_t r8;
    uint64_t r9;

    // --- 非易失性寄存器（用于上下文切换现场保护） ---
    uint64_t rbp;
    uint64_t rbx;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
};
#pragma pack(pop)