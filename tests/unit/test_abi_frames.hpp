#pragma once

#include "test_framework.hpp"
#include <simulator/WinTaskContext.hpp>
#include <simulator/WinX64Regs.hpp>

void unit_test_shadow_space_and_alignment_contract()
{
    // 1. 实例化具体的上下文（不再依赖 CPUEngine）
    WinTaskContext ctx;

    // 2. 准备任务栈
    const size_t STACK_SIZE = 4096;
    // 强制 16 字节对齐分配，排除因 malloc 自身对齐导致测试“撞大运”通过的情况
    void *stack_mem = _aligned_malloc(STACK_SIZE, 16);
    void *stack_top = static_cast<uint8_t *>(stack_mem) + STACK_SIZE;

    // 3. 执行设置流
    // 假设我们要进入 mock_entry，退出时进入 nullptr（或 exit_stub）
    auto mock_entry = []() { /* 模拟执行 */ };
    ctx.setup_flow((void (*)())mock_entry, stack_top);

    // --- 核心校验逻辑 ---

    // 1. 获取 setup_flow 后的栈指针 (RSP)
    // 此时 RSP 指向 WinX64Regs 结构体底部
    uintptr_t rsp_in_context = (uintptr_t)ctx.get_stack_pointer();

    // 2. 模拟汇编执行过程
    // context_load_asm 会 pop 出所有寄存器，最后执行一条 'ret'
    // ret 弹出 8 字节的 RIP，此时 RSP 会增加：sizeof(WinX64Regs)
    uintptr_t rsp_at_entry = rsp_in_context + sizeof(WinX64Regs);

    // --- 校验 A: 16 字节对齐相位 (Phase) ---
    //
    // 规则：在进入函数第一条指令时，RSP 必须满足 (RSP + 8) % 16 == 0
    // 也就是说：RSP % 16 必须等于 8。
    if (rsp_at_entry % 16 != 8)
    {
        _aligned_free(stack_mem);
        throw std::runtime_error("ABI Violation: RSP at entry must be 16n + 8");
    }

    // --- 校验 B: 影子空间 (Shadow Space / Home Space) ---
    // Windows 规定调用者必须预留 32 字节。
    // 这 32 字节位于“返回地址”之上（高地址方向）。
    uintptr_t available_space = (uintptr_t)stack_top - rsp_at_entry;

    // 注意：这里要减去返回地址占用的 8 字节，剩下的才是给函数用的影子空间
    if (available_space < 32)
    {
        _aligned_free(stack_mem);
        throw std::runtime_error("ABI Violation: No shadow space (32 bytes) reserved");
    }

    _aligned_free(stack_mem);
    std::cout << "  [PASS] Shadow Space and Alignment Contract Verified." << std::endl;
}
