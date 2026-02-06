#pragma once

#include "test_framework.hpp"

#include <setjmp.h>
#include <windows.h>
#include <iostream>

extern "C" void context_switch_asm(void **old_sp, void *new_sp);

// 用于跨上下文恢复测试现场
static void *g_main_sp = nullptr;
static bool g_logic_executed = false;
static ITaskContext *g_main_context_ptr = nullptr;

// 1. 定义退出桩 (必须是 C 格式，供 setup_flow 使用)
// 确保 exit_stub 不被内联，并保持最干净的调用约定
__declspec(noinline) void exit_stub()
{
    // 这里不需要通过 dummy 对象
    // 直接使用 g_main_context_ptr 即可
    if (g_main_context_ptr)
    {
        std::cout << "[INFO] Task finished. Returning to Main Context..." << std::endl;

        // 我们需要一个临时的 sp 来存放当前这个“已经完成的任务”的最后状态
        // 否则 context_switch_asm 的第一个参数 [rcx] (old_sp) 没处写
        void *garbage_sp = nullptr;

        // 获取目标 context
        auto *target = static_cast<WinTaskContext *>(g_main_context_ptr);

        // 汇编调用：第一个参数是旧栈保存位置，第二个是新栈地址
        context_switch_asm(&garbage_sp, target->get_stack_pointer());
    }
}

// 2. 增加 noinline 确保它是一个真正的函数调用，拥有标准的 ret 指令
// 增加强制对齐属性，确保 mock 函数符合标准调用约定
__declspec(noinline) void mock_task_entry(void *, void *)
{
    g_logic_executed = true;

    uintptr_t rsp_val;
#ifdef _MSC_VER
    // _AddressOfReturnAddress 给出的是指向返回地址的指针，
    // 在 x64 下，进入函数时 RSP 指向返回地址。
    rsp_val = (uintptr_t)_AddressOfReturnAddress();
#endif

    std::cout << "[INFO] Task Logic Running. Entry RSP: 0x" << std::hex << rsp_val << std::endl;

    // 关键校验：进入函数第一行，RSP 必须是 16n + 8
    // 也就是说，地址结尾必须是 8 (例如 ...8, ...18, ...28)
    if (rsp_val % 16 != 8)
    {
        std::cerr << "[ERROR] ABI Alignment Violation! RSP should be 16n + 8, but is " << (rsp_val % 16) << std::endl;
        // 如果这里崩溃，说明 setup_registers 的减法没算对
    }

    // 模拟一段 K_DEBUG 调用，验证影子空间是否可用
    std::cout << "[INFO] Testing shadow space with complex call..." << std::endl;
}

// 3. 自动化测试用例
void unit_test_context_switch_lifecycle()
{
    AddVectoredExceptionHandler(1, [](PEXCEPTION_POINTERS p) -> LONG
                                {
    printf("\n[FATAL] Exception 0x%X at address %p\n", 
           p->ExceptionRecord->ExceptionCode, p->ExceptionRecord->ExceptionAddress);
    TerminateProcess(GetCurrentProcess(), 1);
    return EXCEPTION_CONTINUE_SEARCH; });

    // 确保主线程有一个合法的 context 容器来保存它被切走时的现场
    WinTaskContext main_ctx(nullptr, 0);

    // 任务上下文：配置 exit_stub
    WinTaskContext task_ctx(exit_stub, 0);
    g_main_context_ptr = &main_ctx;

    const size_t STACK_SIZE = 8192;
    void *stack_mem = _aligned_malloc(STACK_SIZE, 16);
    // 指向最高地址（向下增长的起点）
    void *stack_top = static_cast<uint8_t *>(stack_mem) + STACK_SIZE - 128;

    // 模拟传参：比如传入 runtime = 0x123, config = 0x456
    task_ctx.load_argument(0, 0x123);
    task_ctx.load_argument(1, 0x456);

    // 核心准备：根据我们讨论的 [Shadow][ExitStub][RIP][Regs] 布局初始化
    task_ctx.setup_flow(mock_task_entry, stack_top);

    std::cout << "[INFO] Main Thread: RSP before transit = " << &main_ctx << std::endl;

    // --- 执行切换 ---
    // 此刻，主线程会在这里“暂停”，寄存器存入 main_ctx.sp
    // CPU 跳入 task_ctx.entry_func (即 mock_task_entry)
    main_ctx.transit_to(&task_ctx);

    // --- 当任务运行结束执行 exit_stub 时，代码会神奇地从这里恢复 ---
    std::cout << "[SUCCESS] Back in Main Thread. Logic Executed: " << std::boolalpha << g_logic_executed << std::endl;

    _aligned_free(stack_mem);
}