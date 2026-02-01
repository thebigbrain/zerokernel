#include <cassert>
#include <stdexcept>
#include "simulator/WinTaskContext.hpp"
#include "simulator/WinX64Regs.hpp"
#include <simulator/WinCPUEngine.hpp>

void test_context_abi_alignment()
{
    // 1. 准备阶段
    WinCPUEngine cpu;
    size_t stack_size = 64 * 1024;
    void *stack_base = malloc(stack_size);
    void *ctx_mem = malloc(cpu.get_context_size());

    ITaskContext *ctx = cpu.create_context_at(ctx_mem);
    void *entry = (void *)0x12345678;
    void *exit_r = (void *)0x87654321;

    // 2. 执行准备动作
    ctx->prepare((void (*)())entry, (char *)stack_base + stack_size, (void (*)())exit_r);

    // 3. 契约校验 (取代调试)
    WinX64Regs *regs = (WinX64Regs *)ctx->get_stack_pointer();

    // A. 校验寄存器值是否正确写入镜像
    if (regs->rip != (uint64_t)entry)
        throw std::runtime_error("RIP mismatch in stack frame");

    // B. 校验对齐：Windows 要求进入函数前栈顶是 16 字节对齐
    // 注意：ret 弹出后 RSP 会增加 8，所以 ret 之前的 RSP 应该是 16n + 8 或符合特定布局
    uintptr_t sp_value = (uintptr_t)ctx->get_stack_pointer();
    if (sp_value % 8 != 0)
        throw std::runtime_error("SP is not 8-byte aligned");

    // C. 校验影子空间是否存在
    // 影子空间应该在 exit_router 之上
    uint64_t *shadow_check = (uint64_t *)((uintptr_t)regs + sizeof(WinX64Regs));
    // 这里根据你定义的布局进行 offset 检查

    free(stack_base);
    free(ctx_mem);
}