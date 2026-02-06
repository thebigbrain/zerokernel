#pragma once

#include "test_framework.hpp"
#include <simulator/WinTaskContext.hpp>
#include <simulator/WinX64Regs.hpp>
#include <cassert>
#include <kernel/Memory.hpp>

// --- 测试 1: 验证上下文切换的堆栈布局是否符合 Windows ABI ---
void unit_test_simulator_context_abi()
{
    WinTaskContext ctx;

    // 使用内存对齐分配，模拟 16 字节对齐的真实环境
    void *stack_mem = _aligned_malloc(4096, 16);
    void *stack_top = static_cast<uint8_t *>(stack_mem) + 4096;

    // 预设测试值
    uintptr_t mock_entry = 0xDEADC0DE;
    uintptr_t mock_proxy = 0x11112222;
    uintptr_t mock_config = 0x33334444;

    ctx.load_argument(0, mock_proxy);
    ctx.load_argument(1, mock_config);
    ctx.setup_flow((void (*)(void *, void *))mock_entry, stack_top);

    // --- 校验 1: 参数寄存器镜像 ---
    WinX64Regs *regs = static_cast<WinX64Regs *>(ctx.get_stack_pointer());
    if (regs->rcx != mock_proxy || regs->rdx != mock_config)
    {
        _aligned_free(stack_mem);
        throw std::runtime_error("ABI Error: RCX/RDX parameter mapping failed.");
    }

    // --- 校验 2: 16字节对齐契约 (进入函数瞬间) ---
    uintptr_t sp_in_struct = (uintptr_t)ctx.get_stack_pointer();
    // 模拟 RESTORE: 弹出 96 字节寄存器
    uintptr_t sp_at_rip = sp_in_struct + sizeof(WinX64Regs);
    // 模拟 RET: 弹出 8 字节 Entry 地址
    uintptr_t sp_at_entry = sp_at_rip + 8;

    // Windows ABI 契约：进入函数时 (RSP % 16 == 8)
    if (sp_at_entry % 16 != 8)
    {
        _aligned_free(stack_mem);
        throw std::runtime_error("ABI Violation: RSP alignment at entry must be 16n + 8");
    }

    // --- 校验 3: 影子空间保护区 ---
    // 影子空间必须在返回地址 (ExitStub) 之上
    uintptr_t return_addr_pos = sp_at_entry; // 当前 RSP 指向 ExitStub
    uintptr_t shadow_base = return_addr_pos + 8;

    if (((uintptr_t)stack_top - shadow_base) < 32)
    {
        _aligned_free(stack_mem);
        throw std::runtime_error("ABI Violation: Shadow space missing or overlapped.");
    }

    _aligned_free(stack_mem);
    std::cout << "[SUCCESS] Full ABI Contract Verified. Simulator is stable." << std::endl;
}
// --- 测试 2: 物理内存读写一致性 ---
void unit_test_simulator_memory_layout()
{
    size_t mem_size = 1024 * 1024; // 1MB
    void *base = malloc(mem_size);
    PhysicalMemoryLayout layout{base, mem_size};

    // 验证边界计算
    uint8_t *last_byte = (uint8_t *)layout.base + layout.size - 1;
    *last_byte = 0xAA;

    if (*((uint8_t *)base + 1024 * 1024 - 1) != 0xAA)
    {
        throw std::runtime_error("Memory Layout Error: Address calculation mismatch");
    }

    free(base);
}
