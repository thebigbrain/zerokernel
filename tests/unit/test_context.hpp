#pragma once

#include <cassert>
#include <stdexcept>
#include "simulator/WinTaskContext.hpp"
#include "simulator/WinX64Regs.hpp"
#include <simulator/WinCPUEngine.hpp>

void test_context_abi_alignment()
{
    // 1. 准备阶段 - 修复：直接实例化 WinTaskContext
    WinTaskContext ctx;
    size_t stack_size = 64 * 1024;

    // 使用 _aligned_malloc 确保基地址对齐，排除干扰因素
    void *stack_base = _aligned_malloc(stack_size, 16);
    void *stack_top = (char *)stack_base + stack_size;

    void *entry = (void *)0x12345678;
    void *exit_r = (void *)0x87654321;

    // 2. 执行准备动作
    ctx.setup_flow((void (*)())entry, stack_top, (void (*)())exit_r);

    // 3. 契约校验
    // 获取当前上下文镜像的起始地址
    WinX64Regs *regs = static_cast<WinX64Regs *>(ctx.get_stack_pointer());
    uintptr_t sp_value = reinterpret_cast<uintptr_t>(regs);

    // A. 校验 RIP 是否正确写入结构体
    if (regs->rip != (uint64_t)entry)
        throw std::runtime_error("RIP mismatch in stack frame");

    // B. 校验对齐相位 (The "RSP % 16 == 8" Rule)
    // 逻辑：
    // 1. context_switch_asm 会执行一系列 POP，最后执行 RET。
    // 2. 执行 RET 之前，RSP 指向返回地址（exit_router）。
    // 3. RET 弹出 8 字节后，进入 entry 函数。
    // 4. Windows ABI 要求：进入函数第一条指令时，RSP 必须是 16n + 8。

    // 模拟汇编执行完后的 RSP
    uintptr_t rsp_at_entry = sp_value + sizeof(WinX64Regs);

    if (rsp_at_entry % 16 != 8)
    {
        throw std::runtime_error("ABI Alignment Violation: RSP at entry must be 16n + 8");
    }

    // C. 校验影子空间 (Shadow Space) 完整性
    // 影子空间必须在返回地址 (exit_r) 之上（高地址方向）
    uintptr_t shadow_space_start = rsp_at_entry; // 即返回地址所在位置的上方
    uintptr_t stack_limit = reinterpret_cast<uintptr_t>(stack_top);

    // 计算可用空间（不含返回地址本身的 8 字节）
    if (stack_limit - shadow_space_start < 32)
    {
        throw std::runtime_error("ABI Violation: Missing 32-byte Shadow Space above return address");
    }

    _aligned_free(stack_base);
}
