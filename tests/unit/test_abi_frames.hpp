#pragma once

#include "test_framework.hpp"
#include <simulator/WinTaskContext.hpp>
#include <simulator/WinX64Regs.hpp>

void unit_test_shadow_space_and_alignment_contract()
{
    // 1. 实例化具体的上下文
    WinTaskContext ctx;

    // 2. 准备任务栈
    const size_t STACK_SIZE = 4096;
    // 使用 _aligned_malloc 确保基地址对齐，消除外部干扰
    void *stack_mem = _aligned_malloc(STACK_SIZE, 16);
    void *stack_top = static_cast<uint8_t *>(stack_mem) + STACK_SIZE;

    // 3. 执行设置流
    // 模拟任务入口
    auto mock_entry = [](void *, void *) {};
    ctx.setup_flow(mock_entry, stack_top);

    // --- 核心校验逻辑 ---

    // 获取 setup_flow 后的栈指针 (RSP)
    // 此时 RSP 指向 WinX64Regs 结构体底部（即汇编开始 POP 的起始位置）
    uintptr_t rsp_in_context = (uintptr_t)ctx.get_stack_pointer();

    // 模拟汇编：执行完所有 POP 操作后的 RSP
    // context_load_asm 弹出 WinX64Regs 后，RSP 增加 sizeof(WinX64Regs)
    uintptr_t rsp_after_pops = rsp_in_context + sizeof(WinX64Regs);

    // 此时 RSP 应该正对着 Entry Point 地址，汇编将执行 'ret'
    // ret 弹出 8 字节后，CPU 正式进入 mock_entry
    uintptr_t rsp_at_entry = rsp_after_pops + 8;

    // --- 校验 A: 16 字节对齐契约 ---
    // 规则：在进入函数第一条指令时，RSP 必须是 16 字节对齐的
    // 注意：call 指令压入返回地址后，RSP 会变成 16n + 8。
    // 但我们的 setup_flow 是通过 ret “跳”进去的，模拟的是被 call 后的状态。
    // 契约：Win64 函数期望进入时 (RSP + 8) 是 16 的倍数（即 RSP 结尾为 8）
    K_T_ASSERT(rsp_after_pops % 16 == 8, "ABI Violation: RSP before 'ret' must be 16n + 8");

    // --- 校验 B: 影子空间 (Shadow Space) 契约 ---
    // 规则：在返回地址之上（高地址方向）必须至少有 32 字节可用空间
    // 这块空间用于被调用者保存 RCX, RDX, R8, R9
    uintptr_t shadow_space_start = rsp_after_pops + 8; // 越过返回地址
    uintptr_t available_space = (uintptr_t)stack_top - shadow_space_start;

    K_T_ASSERT(available_space >= 32, "ABI Violation: Shadow space (32 bytes) is missing or insufficient");

    // --- 校验 C: 栈顶边界 ---
    // 确保整个布局没有超出我们分配的内存范围
    K_T_ASSERT(rsp_in_context >= (uintptr_t)stack_mem, "Stack Overflow during setup_flow");

    _aligned_free(stack_mem);
    std::cout << "  [PASS] Shadow Space and Alignment Contract Verified." << std::endl;
}