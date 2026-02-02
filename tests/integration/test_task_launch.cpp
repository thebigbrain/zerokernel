#include <cassert>
#include "kernel/Kernel.hpp"
#include "simulator/WinTaskContext.hpp"
#include "simulator/WinCPUEngine.hpp"
#include "test_framework.hpp"

void actual_dummy_entry() {}
/**
 * 自动化验证任务启动前的所有前置条件
 */
void test_task_preparation_flow()
{
    // 1. 模拟环境准备
    uint8_t *fake_physical_mem = (uint8_t *)malloc(1024 * 1024);
    ObjectFactory factory({fake_physical_mem, 1024 * 1024});

    // 2. 创建核心组件 - 修复：直接使用 WinTaskContext 或其工厂
    // 不再依赖 cpu.create_context_at
    WinTaskContext ctx;

    // 3. 模拟栈和参数准备
    // 修复：确保分配的栈基地址满足 16 字节对齐（ObjectFactory 已支持）
    void *stack_base = factory.allocate_raw(4096);
    void *stack_top = static_cast<char *>(stack_base) + 4096;

    auto dummy_entry = []() { /* 模拟入口 */ };
    auto dummy_exit = []() { /* 模拟退出 */ };

    // 4. 执行准备逻辑
    // 注意：内部会处理 Shadow Space 和对齐
    ctx.setup_flow((void (*)())dummy_entry, stack_top, (void (*)())dummy_exit);

    // 5. 验证点 1：寄存器结构体中的 RIP 是否正确
    WinX64Regs *regs = static_cast<WinX64Regs *>(ctx.get_stack_pointer());
    K_ASSERT(regs->rip == reinterpret_cast<uintptr_t>((void (*)())dummy_entry), "RIP mismatch");

    // 6. 验证点 2：栈对齐深度解析 (Windows x64 ABI)
    uintptr_t sp_val = reinterpret_cast<uintptr_t>(ctx.get_stack_pointer());

    // 物理层验证：
    // (1) 初始时，RSP 必须是 16 字节对齐的 (regs 结构体起始位置)
    K_ASSERT(sp_val % 16 == 0, "Initial RSP (context struct) must be 16-byte aligned");

    // (2) 模拟上下文切换汇编序列：
    // [当前状态] sp_val 指向 WinX64Regs 结构体底部
    // [步骤 1] 汇编执行 10 次以上 POP (不含 RIP) -> RSP 增加 sizeof(WinX64Regs) - 8
    // [步骤 2] 执行 RET 指令弹出 RIP -> RSP 增加 8
    // 此时 RSP 到达 entry 函数的第一条指令位置

    uintptr_t rsp_at_entry_start = sp_val + sizeof(WinX64Regs);

    // 重要：Windows ABI 规定，在进入函数的第一条指令时，(RSP + 8) 必须是 16 的倍数
    // 因为 CALL 指令压入了一个 8 字节的返回地址。
    // 换句话说：RSP % 16 必须等于 8。

    K_ASSERT(rsp_at_entry_start % 16 == 8,
             "ABI Violation: RSP % 16 at entry must be 8, but got " << (rsp_at_entry_start % 16));

    // (3) 验证 Shadow Space (32字节)
    // entry 函数内部会直接访问 [RSP + 8] 到 [RSP + 40]，这部分空间必须在 stack_base 范围内
    uintptr_t shadow_top = rsp_at_entry_start + 32;
    uintptr_t actual_limit = reinterpret_cast<uintptr_t>(stack_top);

    K_ASSERT(shadow_top <= actual_limit, "Stack overflow: No room for Shadow Space");

    free(fake_physical_mem);
    std::cout << "  [PASS] Task Preparation Flow Verified." << std::endl;
}

K_TEST_CASE("Integration: Task Preparation Flow", test_task_preparation_flow);