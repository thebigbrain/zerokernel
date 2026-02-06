#pragma once

#include "test_framework.hpp"
#include <simulator/WinTaskContext.hpp>
#include <simulator/WinX64Regs.hpp>

void unit_test_shadow_space_and_alignment_contract()
{
    WinTaskContext ctx;
    const size_t STACK_SIZE = 4096;
    void *stack_mem = _aligned_malloc(STACK_SIZE, 16);
    void *stack_top = static_cast<uint8_t *>(stack_mem) + STACK_SIZE;

    auto mock_entry = [](void *, void *) {};
    ctx.setup_flow(mock_entry, stack_top);

    // 1. 获取汇编切换前的 SP 指针
    uintptr_t rsp_in_context = (uintptr_t)ctx.get_stack_pointer();

    // 2. 模拟 RESTORE_WIN_X64_CONTEXT：跳过所有通用寄存器
    uintptr_t rsp_before_ret = rsp_in_context + sizeof(WinX64Regs);

    // 3. 模拟 ret 指令：弹出 Entry Point 后的物理 RSP
    // 这是任务函数执行第一条指令时的真实 RSP
    uintptr_t rsp_at_entry = rsp_before_ret + 8;

    // --- 校验 A: 16 字节对齐契约 ---
    // 标准：进入函数时 RSP 结尾必须是 8 (即 16n + 8)
    K_T_ASSERT(rsp_at_entry % 16 == 8, "ABI Violation: RSP must be 16n + 8 at function entry");

    // --- 校验 B: 影子空间 (Shadow Space) 契约 ---
    // 此时 [RSP] 应该是退出桩 (Return Address)
    // 根据标准，在返回地址上方（高地址）必须有 32 字节空间
    uintptr_t shadow_space_base = rsp_at_entry + 8;
    uintptr_t available_space = (uintptr_t)stack_top - shadow_space_base;

    K_T_ASSERT(available_space >= 32, "ABI Violation: Shadow space (32 bytes) is missing above return address");

    // --- 校验 C: 栈边界 ---
    K_T_ASSERT(rsp_in_context >= (uintptr_t)stack_mem, "Stack Overflow");

    _aligned_free(stack_mem);
    std::cout << "  [PASS] Standard x64 ABI Contract Verified." << std::endl;
}
