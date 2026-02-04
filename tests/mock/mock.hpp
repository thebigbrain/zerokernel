#pragma once

#include <cstdint>
#include "test_framework.hpp"

#include "kernel/Kernel.hpp"
#include "kernel/StaticLayoutAllocator.hpp"

// 模拟外部段表数据
ZImgSection mock_sections[2] = {
    {".text", 1, 0, 0x1000, 4096},
    {".data", 2, 0, 0x2000, 1024}};

/**
 * @brief 构造一个合法的 BootInfo 数据镜像
 */
inline BootInfo create_mock_boot_info(void *pool_base, size_t pool_size)
{
    BootInfo info{};
    info.magic = 0xDEADBEEF;
    info.version = 1;

    // 绑定真实的测试函数地址
    info.root_task_entry = [](void *rt, void *arg) { /* root logic */ };
    info.idle_task_entry = [](void *arg) { /* idle logic */ };

    // 资源定位 Mock
    info.config_ptr = nullptr; // 暂时留空
    info.memory_size = (uint64_t)pool_size;

    // 段表镜像 Mock
    info.extra_sections_count = 2;
    info.sections_table = mock_sections;

    return info;
}

Kernel *create_kernel(size_t mem_size = 64 * 1024)
{
    // --- 1. 这里的数组就是 pool 的来源 ---
    // 我们在栈上开辟 16KB 空间作为“物理内存”
    void *memory_pool = ::operator new(mem_size, std::align_val_t{16});

    // --- 2. 提取 base 和 size ---
    void *pool_base = memory_pool;
    size_t pool_size = mem_size;

    // --- 3. 注入到 BootInfo ---
    // 这样 Kernel 就能通过 BootInfo 知道“我有 16KB 内存可用”
    BootInfo boot_info = create_mock_boot_info(pool_base, pool_size);

    // --- 4. 同时注入到物理布局逻辑 ---
    PhysicalMemoryLayout layout{pool_base, pool_size};

    auto static_allocator = StaticLayoutAllocator::create(layout);
    Kernel *kernel = new (static_allocator->allocate(sizeof(Kernel))) Kernel(static_allocator);
    K_ASSERT(kernel != nullptr, "kernel is null");

    BootInfo info = create_mock_boot_info(pool_base, pool_size);

    kernel->set_boot_info(&info);
    kernel->set_context_factory(new MockTaskContextFactory());

    return kernel;
}