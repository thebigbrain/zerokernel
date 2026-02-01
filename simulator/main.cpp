#include <windows.h>
#include <iostream>
#include <fstream>

#include "Simulator.hpp"
#include "WinCPUEngine.hpp"
#include "IdleTask.hpp"

#ifndef IMG_PATH
IMG_PATH "../../OS_FULL_PHYSICAL.img"
#endif

    int main()
{
    // --- 1. 硬件模拟环境初始化 ---
    WinCPUEngine *cpu = new WinCPUEngine();
    PhysicalMemoryLayout layout = setup_memory(128 * 1024 * 1024); // 128MB 物理内存

    BootInfo info;
    load_os_image(IMG_PATH, layout, &info);

    // --- 3. 内核基础设施初始化 ---
    // 在模拟物理内存的头部创建对象工厂
    ObjectFactory *factory = new (layout.base) ObjectFactory(layout);

    // 跳过 Factory 自身占用的内存（或者预留一段内核专属区）
    factory->allocate_raw(sizeof(ObjectFactory));

    // 创建内核实例
    Kernel *kernel = factory->create<Kernel>(cpu, factory);

    // --- 5. 内核冷启动 ---
    // 先初始化总线等基础服务
    kernel->bootstrap(&info);

    std::cout << "[Simulator] System is running..." << std::endl;

    return 0;
}