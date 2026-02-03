#include <windows.h>
#include <iostream>
#include <fstream>

#include "Simulator.hpp"
#include "WinCPUEngine.hpp"
#include "IdleTask.hpp"
#include "WinTaskContextFactory.hpp"

extern "C" void kmain(PhysicalMemoryLayout layout, BootInfo info, ICPUEngine *cpu, ITaskContextFactory *factory);
void run_simulator()
{
    // --- 1. 硬件模拟环境初始化 ---
    WinCPUEngine *cpu = new WinCPUEngine();
    PhysicalMemoryLayout layout = setup_memory(128 * 1024 * 1024); // 128MB 物理内存

    BootInfo info;
    load_os_image(IMG_PATH, layout, &info);

    kmain(layout, info, cpu, new WinTaskContextFactory());

    // --- 3. 内核基础设施初始化 ---
    std::cout << "[Simulator] System is running..." << std::endl;
}

int main()
{
#ifndef IMG_PATH
    IMG_PATH "../../OS_FULL_PHYSICAL.img"
#endif

    try
    {
        // 你的模拟器启动逻辑
        run_simulator();
    }
    catch (const std::exception &e)
    {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "UNKNOWN CRASH" << std::endl;
    }

    return 0;
}
