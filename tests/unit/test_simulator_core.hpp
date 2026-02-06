#pragma once

#include "test_framework.hpp"
#include <simulator/WinTaskContext.hpp>
#include <simulator/WinX64Regs.hpp>
#include <cassert>
#include <kernel/Memory.hpp>

// --- 测试 1: 验证上下文切换的堆栈布局是否符合 Windows ABI ---
void unit_test_simulator_context_abi()
{
    // 1. 准备阶段：直接实例化具体的上下文实现
    WinTaskContext ctx;

    // 2. 准备 16 字节对齐的栈空间
    alignas(16) uint8_t mock_stack[4096];
    void *stack_top = mock_stack + 4096;

    // 模拟参数地址
    void *entry = (void *)(uintptr_t)0xDEADC0DE;
    void *proxy = (void *)(uintptr_t)0x11112222;
    void *config = (void *)(uintptr_t)0x33334444;

    // 3. 预设上下文执行流
    ctx.setup_flow((void (*)(void *, void *))entry, stack_top);

    // 4. 加载符合 Win x64 ABI 的参数 (RCX, RDX)
    ctx.load_argument(0, (uintptr_t)proxy);  // 应存入寄存器镜像的 RCX
    ctx.load_argument(1, (uintptr_t)config); // 应存入寄存器镜像的 RDX

    // 5. 关键校验：通过结构体指针访问内存镜像
    WinX64Regs *regs = static_cast<WinX64Regs *>(ctx.get_stack_pointer());

    // 校验 A: 指令指针 (RIP)
    if (regs->rip != (uint64_t)entry)
        throw std::runtime_error("ABI Error: RIP not set correctly");

    // 校验 B: 参数传递 (RCX, RDX)
    if (regs->rcx != (uint64_t)proxy)
        throw std::runtime_error("ABI Error: Parameter 0 (RCX) mismatch");
    if (regs->rdx != (uint64_t)config)
        throw std::runtime_error("ABI Error: Parameter 1 (RDX) mismatch");

    // 6. 栈对齐检查 (Windows x64 ABI 契约)
    // 逻辑：
    // 当汇编 context_switch 执行完毕并 ret 后，RSP 会增加 sizeof(WinX64Regs)
    uintptr_t final_sp = (uintptr_t)ctx.get_stack_pointer();
    uintptr_t rsp_at_entry = final_sp + sizeof(WinX64Regs);

    //

    // 契约：在进入函数的第一条指令时，RSP % 16 必须等于 8
    // (因为 call 指令压入了一个 8 字节的返回地址，打破了原始的 16 字节对齐)
    if (rsp_at_entry % 16 != 8)
    {
        throw std::runtime_error("ABI Error: Stack alignment violates Win x64 convention. "
                                 "RSP % 16 is " +
                                 std::to_string(rsp_at_entry % 16));
    }

    // 7. 额外校验：影子空间 (Shadow Space) 检查
    // 从 rsp_at_entry 到 stack_top 之间必须有至少 32 字节的空白区
    uintptr_t stack_limit = (uintptr_t)stack_top;
    if (stack_limit - rsp_at_entry < 32)
    {
        throw std::runtime_error("ABI Error: Shadow space (32 bytes) is missing!");
    }

    std::cout << "  [PASS] Simulator Context ABI Verified." << std::endl;
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
