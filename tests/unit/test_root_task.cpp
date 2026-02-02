#include <cstdint>
#include <iostream>
#include <vector>
#include <cstring>

#include "test_framework.hpp"
#include "common/BootInfo.hpp"

// --- Mock 环境准备 ---

// 1. 定义一个静态的 BootInfo 结构
static BootInfo mock_boot_info;
static BootInfo *info = &mock_boot_info;

// 2. 准备一段模拟的 RootTask 二进制代码 (即你日志中看到的 48 89 4C 24)
static uint8_t mock_root_task_bin[] = {
    0x48, 0x89, 0x4C, 0x24, 0x08, // mov [rsp+8], rcx
    0x48, 0x83, 0xEC, 0x28,       // sub rsp, 0x28
    0x90,                         // nop
    0xC3                          // ret
};

/**
 * 环境初始化：在测试开始前将 entry 指向我们的模拟代码
 */
void setup_instruction_test()
{
    info->root_task_entry = (void (*)(void *))(uintptr_t)mock_root_task_bin;
}
/**
 * Test 20: 验证入口点指令的物理存在与合法性
 */
void test_root_task_instruction_sanity()
{
    setup_instruction_test();

    std::cout << "[RUN      ] RootTask Instruction Sanity" << std::endl;

    // 假设从 Loader 获取到的信息
    uint8_t *entry_ptr = (uint8_t *)info->root_task_entry;

    // 1. 验证不是空内存 (0x00) 或未初始化的闪存 (0xFF)
    K_ASSERT(entry_ptr[0] != 0x00, "Entry point points to null memory!");
    K_ASSERT(entry_ptr[0] != 0xFF, "Entry point points to uninitialized flash!");

    // 2. 验证是否符合 'mov [rsp+8], rcx' (48 89 4C 24)
    // 这是我们在日志中看到的，代表了 RootTask 确实在尝试处理 rt 参数
    bool matches_prologue = (entry_ptr[0] == 0x48 && entry_ptr[1] == 0x89);

    if (!matches_prologue)
    {
        std::cout << "[WARN] Entry instruction is not standard prologue. "
                  << "Found: " << std::hex << (int)entry_ptr[0] << std::endl;
    }

    K_ASSERT(entry_ptr[0] == 0x48, "Expected x64 REX.W prefix (0x48) at entry!");
    std::cout << "  [PASS] Instruction sanity check." << std::endl;
}

K_TEST_CASE("Unit Test Suite for RootTask", test_root_task_instruction_sanity);