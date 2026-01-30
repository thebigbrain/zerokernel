#include <windows.h>
#include <iostream>
#include <fstream>
#include "kernel/Kernel.hpp"
#include "WinCPUEngine.hpp"
#include "kernel/Memory.hpp"
#include <common/BootInfo.hpp>
#include "IdleTask.hpp"

// 模拟器内存初始化
PhysicalMemoryLayout setup_memory(size_t size)
{
    PhysicalMemoryLayout layout;
    layout.base = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    layout.size = size;
    return layout;
}

BootInfo *prepare_boot_info(PhysicalMemoryLayout &layout)
{
    // 2. 约定：BootInfo 结构体放在内存起始后的第一个 4KB 处
    // (假设 0-4KB 是内核代码，4KB-8KB 是 BootInfo)
    BootInfo *info = (BootInfo *)((uintptr_t)layout.base + 0x1000);

    // 3. 填入物理镜像中各组件的真实运行地址
    info->magic = 0xDEADBEEF;
    info->root_task_entry = (void (*)(void *, void *))((uintptr_t)layout.base + 0x1000000); // 16MB
    info->idle_task_entry = (void (*)(void *))task_idle_entry;                              // 内核导出的函数
    info->config_ptr = (void *)((uintptr_t)layout.base + 0x2000000);

    return info;
}

int main()
{
    // --- 1. 硬件模拟环境初始化 ---
    WinCPUEngine *cpu = new WinCPUEngine();
    PhysicalMemoryLayout layout = setup_memory(128 * 1024 * 1024); // 128MB 物理内存

    BootInfo *info = prepare_boot_info(layout);

    // --- 2. 加载物理镜像 (OS_FULL_PHYSICAL.img) ---
    // 模拟 Bootloader 将打包好的镜像读入模拟内存基址
    std::ifstream img("../../OS_FULL_PHYSICAL.img", std::ios::binary);
    if (img.is_open())
    {
        img.read((char *)layout.base, layout.size);
        img.close();
        std::cout << "[Simulator] OS Image loaded into memory." << std::endl;
    }
    else
    {
        std::cerr << "[Error] OS_FULL_PHYSICAL.img not found!" << std::endl;
        return -1;
    }

    // --- 3. 内核基础设施初始化 ---
    // 在模拟物理内存的头部创建对象工厂
    ObjectFactory *factory = new (layout.base) ObjectFactory(layout);

    // 跳过 Factory 自身占用的内存（或者预留一段内核专属区）
    factory->allocate_raw(sizeof(ObjectFactory));

    // 创建内核实例
    Kernel *kernel = factory->create<Kernel>(cpu, factory);

    // --- 5. 内核冷启动 ---
    // 先初始化总线等基础服务
    kernel->bootstrap(info);

    std::cout << "[Simulator] System is running..." << std::endl;

    return 0;
}