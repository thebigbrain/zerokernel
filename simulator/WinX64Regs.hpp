#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct WinX64Regs
{
    // 这里的顺序必须严格对应 context_switch_asm 中 pop 的反向顺序
    // 假设你的汇编是：pop rbp, rbx, rdi, rsi, r12-r15, rdx, rcx
    uint64_t rcx; // sp 指向这里
    uint64_t rdx;
    uint64_t r8; // 建议加上，符合 Windows x64 传参规范
    uint64_t r9; // 建议加上
    uint64_t r15, r14, r13, r12, rsi, rdi, rbx, rbp;

    uint64_t rip; // 结构体末尾，地址最高
};
#pragma pack(pop)