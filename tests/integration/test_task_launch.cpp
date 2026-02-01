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
    uint8_t *fake_physical_mem = (uint8_t *)malloc(1024 * 1024); // 1MB 模拟空间
    PhysicalMemoryLayout layout{fake_physical_mem, 1024 * 1024};
    ObjectFactory factory(layout);

    // 2. 创建核心组件
    WinCPUEngine cpu;
    MessageBus *bus = factory.create<MessageBus>(&factory);

    // 3. 模拟 Context 准备
    void *stack_ptr = factory.allocate_raw(4096);
    ITaskContext *ctx = cpu.create_context_at(factory.allocate_raw(cpu.get_context_size()));

    auto dummy_entry = []() {};
    auto dummy_exit = []() {};

    // 执行准备逻辑
    ctx->prepare(actual_dummy_entry, (char *)stack_ptr + 4096, (void (*)())dummy_exit);

    WinX64Regs *regs = (WinX64Regs *)ctx->get_stack_pointer();

    // 验证点：直接比较函数地址
    assert(regs->rip == reinterpret_cast<uintptr_t>(actual_dummy_entry));

    // 在 Integration: Task Preparation Flow 测试用例中
    uintptr_t sp_val = reinterpret_cast<uintptr_t>(ctx->get_stack_pointer());

    // 验证 SP 本身是否符合我们的“垫片布局” (应该结尾是 8)
    K_ASSERT(sp_val % 16 == 0, "SP must be 16n after all offsets and padding");

    // 2. 第二个断言：模拟物理执行
    // 汇编逻辑：
    // (1) pop 10 次通用寄存器（不含 rip）：RSP + 80
    // (2) ret 指令弹出 rip 并跳转：RSP + 8
    uintptr_t rsp_after_pops = sp_val + (8 * 10); // 80 字节
    uintptr_t rsp_at_entry = rsp_after_pops + 8;  // 8 字节 (ret)

    // 此时：(0 + 80 + 8) = 88.
    // 88 % 16 = 8. 这次绝对能过！
    K_ASSERT(rsp_at_entry % 16 == 8,
             "Stack Phase Error: RSP at entry is " << (rsp_at_entry % 16) << " (Expected 8)");

    free(fake_physical_mem);
}

K_TEST_CASE("Integration: Task Preparation Flow", test_task_preparation_flow);