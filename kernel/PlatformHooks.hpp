#pragma once

#include <cstdint>
#include "ISchedulingControl.hpp"
#include "ITaskContextFactory.hpp"
#include "ISignal.hpp"

/**
 * @brief 平台抽象集合
 * 在 kmain 启动时，由平台层填充并注入给内核
 */
struct PlatformHooks
{
    ISchedulingControl *sched_control;
    ISignalGate *dispatcher;
    ITaskContextFactory *task_context_factory;

    // 平台相关的基础行为
    void (*reboot)();
    void (*halt)();

    // 内存相关的平台特性
    void *(*get_initial_heap_base)();
};