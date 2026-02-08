#include <windows.h>
#include <iostream>
#include <fstream>

#include "Simulator.hpp"
#include "IdleTask.hpp"
#include "WinTaskContextFactory.hpp"
#include "Win32SignalGate.hpp"
#include "Win32SchedulingControl.hpp"
#include <kernel/PlatformHooks.hpp>
#include "LoggerWin.hpp"

extern "C" void kmain(PhysicalMemoryLayout layout,
                      BootInfo info,
                      PlatformHooks *platform_hooks);

extern ISchedulingControl *g_platform_sched_ctrl;

void run_simulator()
{
    // --- 1. 硬件模拟环境初始化 ---
    PhysicalMemoryLayout layout = setup_memory(128 * 1024 * 1024); // 128MB 物理内存

    BootInfo info;
    load_os_image(IMG_PATH, layout, &info);

    auto *signal_dispatcher = new Win32SignalGate();
    auto *sched_control = new Win32SchedulingControl(signal_dispatcher);

    g_platform_sched_ctrl = sched_control;

    PlatformHooks hooks;
    hooks.dispatcher = signal_dispatcher;
    hooks.sched_control = sched_control;
    hooks.task_context_factory = new WinTaskContextFactory();
    hooks.halt = []()
    {
        klog(LogLevel::Info, "Halt");
        Sleep(5000);
    };

    kmain(layout, info, &hooks);

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
