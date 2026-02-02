#include <iostream>
#include <cstdint>
#include "kernel/ObjectFactory.hpp"
#include "simulator/WinTaskContext.hpp"
#include "test_framework.hpp"
#include <assert.h>

// 全局状态追踪
static bool g_root_task_executed = false;
static uint64_t g_magic_check = 0;
static void *g_saved_kernel_sp = nullptr;
static void *g_saved_task_sp = nullptr; // 存储任务现场（用于下次切回任务）

// 汇编切换函数声明
extern "C" void context_switch_asm(void **old_sp, void *new_sp);

/**
 * @brief 任务退出路由器
 * 任务 ret 后落在此处，负责切回内核主流程
 */
// 确保这个函数绝对不会被优化掉
extern "C" void root_task_exit_router()
{
    std::cout << "  [DEBUG] In Router!" << std::endl; // 看能不能打印出来
    // 任务已经执行完了 root_task_entry 的逻辑
    // 现在我们要利用保存好的内核 SP，跳回内核 test 函数的断言处
    if (g_saved_kernel_sp)
    {
        context_switch_asm(&g_saved_task_sp, g_saved_kernel_sp);
    }

    // 如果走到这里，说明切回失败，死循环防止程序跑飞
    while (1)
        ;
}

/**
 * @brief Root Task 入口
 */
// 必须是 extern "C"，且绝对不能是类成员函数
// 裸机环境下真正的 entry 应该长这样
extern "C" void root_task_entry(void *arg)
{
    g_root_task_executed = true;
    g_magic_check = 0x5555AAAA5555AAAAULL;
    // 在裸机上我们可以 return，但在宿主机测试时，我们强制跳回
    // 避免宿主机去执行那个可能被踩坏的、或者不对齐的 ret 逻辑
    context_switch_asm(&g_saved_task_sp, g_saved_kernel_sp);
}

/**
 * @brief 核心集成测试
 */
void test_root_task_lifecycle()
{
    // 0. 重置全局状态 (关键：防止上一个测试用例的影响)
    g_root_task_executed = false;
    g_magic_check = 0;
    g_saved_kernel_sp = nullptr;
    g_saved_task_sp = nullptr;

    // 1. 准备模拟物理内存 (对齐 4KB，更贴近裸机页对齐)
    alignas(4096) static uint8_t fake_phys_mem[256 * 1024];
    PhysicalMemoryLayout layout{fake_phys_mem, sizeof(fake_phys_mem)};

    std::cout << "  [KERNEL] Constructing factory at: " << (void *)layout.base << std::endl;
    ObjectFactory kernel_factory(layout);

    // 2. 创建上下文和栈
    auto *ctx = kernel_factory.create<WinTaskContext>();

    // 2.1 强行隔离（增加缓冲区，防止栈溢出踩坏 Context 对象）
    kernel_factory.allocate_raw(16384);

    size_t stack_size = 32768; // 32KB 栈空间，足以应对 Debug 模式下的复杂调用
    void *stack_bottom = kernel_factory.allocate_raw(stack_size);

    // 关键：强制 stack_top 16 字节对齐
    uintptr_t top_addr = (reinterpret_cast<uintptr_t>(stack_bottom) + stack_size) & ~0xFULL;
    void *stack_top = reinterpret_cast<void *>(top_addr);

    // 3. 准备任务 (注入入口和路由器)
    // 此时执行 prepare，内部会进行 8 字节相位补偿
    ctx->setup_flow(reinterpret_cast<void (*)()>(root_task_entry), stack_top, root_task_exit_router);

    // 4. 参数注入
    ctx->load_argument(0, reinterpret_cast<uintptr_t>(&kernel_factory));
    ctx->load_argument(1, 0x2222);
    ctx->load_argument(2, 0x3333);
    ctx->load_argument(3, 0x4444);

    // 5. 打印关键调试信息 (用于断言失败时的手动核对)
    std::cout << "  [DEBUG] Context Addr: " << ctx << std::endl;
    std::cout << "  [DEBUG] Stack Top:    " << stack_top << std::endl;

    // RDX 指向的是寄存器镜像的首地址
    void *rdx_val = ctx->get_stack_pointer();
    std::cout << "  [DEBUG] RSP Init (RDX): " << rdx_val << std::endl;

    // 物理规格校验：验证跳转前 RSP 的相位
    // 88(regs) + 8(padding) + 8(ret) = 104; 104 % 16 = 8.
    // 进入 entry 瞬间的 RSP 应该是 16n + 8
    uintptr_t rsp_at_entry = reinterpret_cast<uintptr_t>(rdx_val) + sizeof(WinX64Regs) + 16;
    if (rsp_at_entry % 16 != 8)
    {
        std::cout << "  [WARNING] ABI Alignment potential issue: " << (rsp_at_entry % 16) << std::endl;
    }

    // 6. 起飞
    std::cout << "  [KERNEL] Launching Root Task..." << std::endl;

    // 执行切换
    context_switch_asm(&g_saved_kernel_sp, rdx_val);

    // --- 回到了内核上下文 ---
    std::cout << "  [BACK] Successfully returned to kernel thread." << std::endl;

    // 7. 最终物理结果断言
    K_ASSERT(g_root_task_executed, "Root Task execution flag not set");
    K_ASSERT(g_magic_check == 0x5555AAAA5555AAAAULL, "Memory integrity failed");

    std::cout << "  [TEST] All systems nominal. Root Task lifecycle verified." << std::endl;
}

/**
 * @brief 验证 WinTaskContext 是否生成了符合裸机 ABI 规范的栈布局
 */
void test_root_task_baremetal_layout()
{
    // 1. 模拟物理内存环境
    alignas(16) static uint8_t fake_stack[4096];
    void *stack_top = fake_stack + 4096;

    // 假设的函数地址
    auto mock_entry = reinterpret_cast<void (*)()>(0x1111222233334444ULL);
    auto mock_router = reinterpret_cast<void (*)()>(0xAAAABBBBCCCCDDD0ULL);

    WinTaskContext ctx;
    ctx.load_argument(0, 0x5555666677778888ULL); // 设置参数 (RCX)
    ctx.setup_flow(mock_entry, stack_top, mock_router);

    // 2. 获取 setup_flow 后的栈指针 (模拟汇编中的 RDX)
    uintptr_t sp_val = reinterpret_cast<uintptr_t>(ctx.get_stack_pointer());

    // --- 模拟 CPU 行为验证 ---

    // A. 验证寄存器镜像是否正确 (基于 sp_val 指向结构体开头)
    auto *regs = reinterpret_cast<WinX64Regs *>(sp_val);
    K_ASSERT(regs->rip == (uintptr_t)mock_entry, "RIP injection failed");
    K_ASSERT(regs->rcx == 0x5555666677778888ULL, "Parameter (RCX) failed");

    // B & C. 模拟汇编执行过程：
    // context_load_asm 会 pop 掉所有寄存器 (除了 rip)，然后 ret 弹出 rip。
    // 这意味着进入函数瞬间，RSP 刚好移动了整个结构体的大小。
    uintptr_t rsp_at_entry_start = sp_val + sizeof(WinX64Regs);

    // D. 【核心断言 1】：进入函数时的栈对齐
    // 104 字节的结构体：如果 sp_val 是 16n，则 16n + 104 = 16n + 8。完美！
    K_ASSERT(rsp_at_entry_start % 16 == 8,
             "Baremetal Alignment Error: RSP mod 16 is " << (rsp_at_entry_start % 16));

    // E. 【核心断言 2】：影子空间验证
    // 根据 Windows ABI，[RSP + 32] 处应该是 Caller 压入的返回地址。
    // 在我们的 setup_flow 逻辑中，这里存放的是 mock_router。
    uintptr_t router_ptr_addr = rsp_at_entry_start + 32;

    // 安全起见，打印一下地址
    // std::cout << "  [DEBUG] RSP at entry: " << (void*)rsp_at_entry_start << std::endl;
    // std::cout << "  [DEBUG] Expected Router at: " << (void*)router_ptr_addr << std::endl;

    uintptr_t exit_router_addr_in_stack = *reinterpret_cast<uintptr_t *>(router_ptr_addr);
    K_ASSERT(exit_router_addr_in_stack == (uintptr_t)mock_router,
             "Shadow space or Exit Router location mismatch. Found: " << (void *)exit_router_addr_in_stack);

    std::cout << "  [PASS] Baremetal stack layout verified." << std::endl;
}

/**
 * @brief 裸机规格验证：验证栈布局是否严格符合 x86_64 调用约定
 */
void test_baremetal_stack_spec()
{
    // 1. 准备隔离的测试空间
    alignas(16) static uint8_t mock_phys_stack[4096];
    void *stack_top = mock_phys_stack + 4096;

    // 模拟两个物理地址（不要真的调用它们）
    auto f_entry = reinterpret_cast<void (*)()>(0xDEADEAD000000001ULL);
    auto f_exit = reinterpret_cast<void (*)()>(0xBBBBBBBB00000002ULL);

    WinTaskContext ctx;
    ctx.load_argument(0, 0xCAFEBABECAFEBABEULL); // 参数注入 (RCX)
    ctx.setup_flow(f_entry, stack_top, f_exit);

    uintptr_t sp_val = reinterpret_cast<uintptr_t>(ctx.get_stack_pointer());

    // --- 校验二：模拟汇编恢复过程 ---
    // 汇编 pop 了 12 次，然后 ret (又 pop 了一次)
    // 所以 RSP 移动了 13 * 8 = 104 字节，正好是 sizeof(WinX64Regs)
    uintptr_t rsp_at_entry_point = sp_val + sizeof(WinX64Regs);

    std::cout << "  [DEBUG] RSP at Entry: " << (void *)rsp_at_entry_point << std::endl;

    // --- 校验三：对齐校验 ---
    // 进入函数时必须是 16n + 8
    K_ASSERT(rsp_at_entry_point % 16 == 8,
             "ABI Violation: RSP must be 16n+8. Actual mod 16: " << (rsp_at_entry_point % 16));

    // --- 校验四：影子空间之后是返回地址 ---
    uintptr_t return_addr_pos = rsp_at_entry_point + 32;

    // 安全检查
    K_ASSERT(return_addr_pos < (uintptr_t)stack_top, "Stack Overflow: Return address beyond top");

    uintptr_t actual_exit_router = *reinterpret_cast<uintptr_t *>(return_addr_pos);
    K_ASSERT(actual_exit_router == (uintptr_t)f_exit, "Exit router address mismatch");

    std::cout << "  [SPEC-OK] Baremetal stack layout verified." << std::endl;
}

void test_after_root()
{
    std::cout << "  [CHECK] If you see this, the test suite is still alive." << std::endl;
}

K_TEST_CASE("Kernel: Baremetal Stack Spec", test_baremetal_stack_spec);
K_TEST_CASE("Kernel: Root Task Baremetal Layout", test_root_task_baremetal_layout);
K_TEST_CASE("Kernel: Root Task Full Lifecycle", test_root_task_lifecycle);
K_TEST_CASE("Z_Final_Check", test_after_root);