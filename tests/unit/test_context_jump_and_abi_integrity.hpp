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
void exit_stub()
{
    std::cout << "[INFO] Exit Stub Called. Transiting back..." << std::endl;

    // 获取当前正在运行的任务上下文（这部分取决于你的调度器实现）
    // 这里我们假设能拿到当前任务，并切回主线程
    // WinTaskContext current_task(nullptr, 0); // 这是一个占位符，实际需要拿到当前任务实例

    if (g_main_context_ptr)
    {
        // 我们需要一个临时的 context 对象来承载“当前任务”的身份
        // 否则汇编代码在保存当前 RSP 时没有目的地
        WinTaskContext dummy_current_task(nullptr, 0);

        // 关键：从 dummy 切回 main
        // 这会把任务最后的栈指针存入 dummy.sp，然后恢复主线程的 sp
        dummy_current_task.transit_to(g_main_context_ptr);
    }
}

// 2. 增加 noinline 确保它是一个真正的函数调用，拥有标准的 ret 指令
void mock_task_entry(void *, void *)
{
    g_logic_executed = true;
    // 简单的逻辑判断：如果能运行到这里不崩，说明对齐和影子空间大概率对了
    std::cout << "[INFO] Task Logic Running..." << std::endl;

    void *current_rsp;
// 获取当前 RSP 指针
#ifdef _MSC_VER
    current_rsp = (void *)_AddressOfReturnAddress();
#endif
    std::cout << "[INFO] Task Logic Running. Current RSP: " << current_rsp << std::endl;
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

    // 创建两个上下文：一个是我们要测的任务，一个是用来承接返回的主线程
    WinTaskContext main_ctx(nullptr, 0);
    WinTaskContext task_ctx(exit_stub, 32);

    g_main_context_ptr = &main_ctx; // 让 exit_stub 知道往哪回
    g_logic_executed = false;

    // 配置任务栈
    const size_t STACK_SIZE = 8192;
    void *stack_mem = _aligned_malloc(STACK_SIZE, 16);
    void *stack_top = static_cast<uint8_t *>(stack_mem) + 4096;
    task_ctx.setup_flow(mock_task_entry, stack_top);

    std::cout << "[INFO] Main Thread transiting to Task..." << std::endl;

    // 在测试函数中
    std::cout << "[DEBUG] Stack Top (High): " << stack_top << std::endl;
    std::cout << "[DEBUG] Stack Bottom (Low): " << stack_mem << std::endl;

    // --- 执行切换 ---
    // transit_to 会把主线程当前的寄存器存入 main_ctx.sp
    // 然后加载 task_ctx.sp
    main_ctx.transit_to(&task_ctx);

    // --- 任务执行 exit_stub 后，会 transit_to(main_ctx)，代码闪现回到这里 ---
    std::cout << "[INFO] Control flow recovered to Main Thread via transit_to." << std::endl;

    K_T_ASSERT(g_logic_executed, "Test Failed");
    _aligned_free(stack_mem);
}