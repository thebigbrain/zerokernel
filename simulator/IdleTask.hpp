#pragma once

#include "common/IUserRuntime.hpp"
#include <windows.h> // 模拟器环境下使用 Sleep

extern "C" void task_idle_entry(IUserRuntime *rt)
{
    while (true)
    {
        // 在模拟器中通过 Sleep 让出宿主 CPU 时间，防止单核 100% 占用
        Sleep(1);

        // 尝试触发调度，看看有没有新任务进入就绪队列
        rt->yield();
    }
}