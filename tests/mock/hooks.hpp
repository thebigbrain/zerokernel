#pragma once
#include <iostream>
#include <vector>
#include <windows.h>

#include <kernel/ISchedulingControl.hpp>
#include <kernel/ISignal.hpp>
#include <kernel/PlatformHooks.hpp>

#include "MockTaskContext.hpp"
#include "MockTaskContextFactory.hpp"
#include "MockSchedulingControl.hpp"
#include "MockSignalDispatcher.hpp"

// 实例化
static MockSchedulingControl g_mock_sched;
static MockSignalDispatcher g_mock_dispatcher;
static MockTaskContextFactory g_mock_factory;

PlatformHooks *create_mock_platform()
{
    auto *hooks = new PlatformHooks();

    // 注入对象实例
    hooks->sched_control = &g_mock_sched;
    hooks->dispatcher = &g_mock_dispatcher;
    hooks->task_context_factory = &g_mock_factory;
    hooks->print = [](const char *msg, PRINT_LEVEL level)
    { std::cout << msg << std::endl; };

    return hooks;
}

void destroy_mock_platform(PlatformHooks *hooks)
{
    delete hooks;
}