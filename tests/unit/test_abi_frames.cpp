#include "test_framework.hpp"
#include <simulator/WinCPUEngine.hpp>
#include <simulator/WinTaskContext.hpp>
#include <simulator/WinX64Regs.hpp>

void test_shadow_space_and_alignment_contract()
{
    WinCPUEngine cpu;
    void *ctx_mem = malloc(cpu.get_context_size());
    ITaskContext *ctx = cpu.create_context_at(ctx_mem);

    // 准备任务栈
    const size_t STACK_SIZE = 4096;
    uint8_t *stack_mem = (uint8_t *)malloc(STACK_SIZE);
    void *stack_top = stack_mem + STACK_SIZE; // 栈顶

    // 模拟入口
    auto mock_entry = []() {};
    ctx->prepare((void (*)())mock_entry, stack_top, nullptr);

    // --- 核心校验逻辑 ---

    // 1. 获取 prepare 后的栈指针 (RSP)
    uintptr_t rsp = (uintptr_t)ctx->get_stack_pointer();

    // 2. 模拟 context_load_asm 执行完 'ret' 后的状态
    // 当 ret 执行时，它会从栈顶弹出一个值到 RIP。
    // 所以在任务的入口点第一条指令时，实际的 RSP 应该是：
    uintptr_t rsp_at_entry = rsp + 8;

    // 校验 A: 16 字节对齐
    // Windows ABI 规定：在 call 指令执行前，RSP 必须 16 字节对齐。
    // 意味着进入函数（call 压入 8 字节后）时，RSP 应该是 16n + 8。
    if (rsp_at_entry % 16 != 8)
    {
        throw std::runtime_error("ABI Violation: RSP at entry must be 16n + 8 for alignment");
    }

    // 校验 B: 影子空间（Home Space）
    // Windows 规定调用者必须在栈上预留 32 字节给被调用者保存 RCX, RDX, R8, R9。
    // 即从 (rsp_at_entry) 到 (stack_top) 必须至少有 32 字节。
    uintptr_t available_space = (uintptr_t)stack_top - rsp_at_entry;
    if (available_space < 32)
    {
        throw std::runtime_error("ABI Violation: No shadow space (32 bytes) reserved on stack");
    }

    free(stack_mem);
    free(ctx_mem);
}

K_TEST_CASE("ABI: Shadow Space & Alignment", test_shadow_space_and_alignment_contract);