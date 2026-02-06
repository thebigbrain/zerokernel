#pragma once

#include <cstdint>
#include "test_framework.hpp"

#include "kernel/Kernel.hpp"
#include "kernel/StaticLayoutAllocator.hpp"

#include "mock/hooks.hpp"

/**
 * @brief Mock: 内核模拟器
 * 封装了物理内存、引导逻辑，并通过 Inspector 提供内核状态访问
 */
class Mock
{
private:
    void *_ram_base = nullptr;
    size_t _ram_size = 0;
    Kernel *_kernel = nullptr;

    PlatformHooks *_platform_hooks = nullptr;
    BootInfo _boot_info;

public:
    Mock(size_t mem_size) : _ram_size(mem_size), _kernel(nullptr)
    {
        _platform_hooks = create_mock_platform();

        // 1. 模拟物理内存申请
        _ram_base = ::operator new(mem_size, std::align_val_t{16});
        _boot_info = create_mock_boot_info(_ram_base, _ram_size);

        // 2. 物理布局与内核实例化
        PhysicalMemoryLayout layout{_ram_base, _ram_size};
        auto static_allocator = StaticLayoutAllocator::create(layout);

        void *k_mem = static_allocator->allocate(sizeof(Kernel));
        _kernel = new (k_mem) Kernel(static_allocator, _boot_info, _platform_hooks);
    }

    ~Mock()
    {
        if (_kernel)
            _kernel->~Kernel();
        if (_ram_base)
            ::operator delete(_ram_base, std::align_val_t{16});

        if (_platform_hooks)
            destroy_mock_platform(_platform_hooks);
    }

    const BootInfo &get_boot_info()
    {
        return _boot_info;
    }

    void *get_ram_start()
    {
        return _ram_base;
    }

    // --- 模拟器对外接口 ---

    // 获取内核实例进行操作
    Kernel *kernel() { return _kernel; }

    // 硬件级信息
    size_t total_ram() const { return _ram_size; }

private:
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
        info.idle_task_entry = [](void *rt, void *arg) { /* idle logic */ };

        // 资源定位 Mock
        info.config_ptr = nullptr; // 暂时留空
        info.memory_size = (uint64_t)pool_size;

        // 段表镜像 Mock
        info.extra_sections_count = 2;
        info.sections_table = mock_sections;

        return info;
    }
};