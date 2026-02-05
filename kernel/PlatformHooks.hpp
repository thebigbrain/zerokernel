#pragma once
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
    ISignalDispatcher *dispatcher;
    ITaskContextFactory *task_context_factory;

    // 平台相关的基础行为
    void (*panic)(const char *msg);
    void (*reboot)();

    // 内存相关的平台特性
    void *(*get_initial_heap_base)();
};