#include "test.hpp"
#include <simulator/WinCPUEngine.hpp>
#include <simulator/WinTaskContext.hpp>
#include <simulator/WinX64Regs.hpp>
#include <cassert>
#include <kernel/Memory.hpp>

// --- 测试 1: 验证上下文切换的堆栈布局是否符合 Windows ABI ---
void test_simulator_context_abi()
{
    WinCPUEngine cpu;
    size_t ctx_size = cpu.get_context_size();
    void *ctx_mem = malloc(ctx_size);

    // 准备一段 16 字节对齐的栈空间
    alignas(16) uint8_t mock_stack[4096];
    void *stack_top = mock_stack + 4096;

    ITaskContext *ctx = cpu.create_context_at(ctx_mem);

    // 模拟参数
    void *entry = (void *)0xDEADC0DE;
    void *proxy = (void *)0x11112222;
    void *config = (void *)0x33334444;

    // 预设上下文
    ctx->prepare((void (*)())entry, stack_top, nullptr);
    ctx->set_parameter(0, (uintptr_t)proxy);  // RCX
    ctx->set_parameter(1, (uintptr_t)config); // RDX

    // 关键校验：直接通过结构体指针访问内存，验证位置
    WinX64Regs *regs = (WinX64Regs *)ctx->get_stack_pointer();

    if (regs->rip != (uint64_t)entry)
        throw std::runtime_error("ABI Error: RIP not set correctly");
    if (regs->rcx != (uint64_t)proxy)
        throw std::runtime_error("ABI Error: Parameter 0 (RCX) mismatch");
    if (regs->rdx != (uint64_t)config)
        throw std::runtime_error("ABI Error: Parameter 1 (RDX) mismatch");

    // 栈对齐检查：Windows 要求在进入函数第一条指令前，RSP+8 必须是 16 字节对齐的
    uintptr_t final_sp = (uintptr_t)ctx->get_stack_pointer();

    uintptr_t rsp_after_pops = final_sp + 80;    // 10个寄存器
    uintptr_t rsp_at_entry = rsp_after_pops + 8; // ret 弹出 rip

    if (rsp_at_entry % 16 != 8)
    {
        throw std::runtime_error("ABI Error: Stack alignment violates Win x64 convention");
    }

    free(ctx_mem);
}

// --- 测试 2: 物理内存读写一致性 ---
void test_simulator_memory_layout()
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

// 注册测试用例
K_TEST_CASE("Simulator: Context ABI Integrity", test_simulator_context_abi);
K_TEST_CASE("Simulator: Physical Memory Map", test_simulator_memory_layout);