#pragma once

#include <kernel/ISchedulingControl.hpp>
#include <kernel/SignalType.hpp>
#include <synchapi.h>
#include <processthreadsapi.h>
#include "Win32SignalGate.hpp"

class Win32SchedulingControl : public ISchedulingControl
{
private:
    HANDLE _kernel_thread_event; // 用于唤醒内核管理线程

    Win32SignalGate *_dispatcher; // 持有分发器引用

public:
    Win32SchedulingControl(Win32SignalGate *dispatcher)
        : _dispatcher(dispatcher)
    {
        _kernel_thread_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    }

    void yield_current_task() override
    {
        // 主动触发一个 Trap 类型的信号，ID 约定为 Yield
        _dispatcher->trigger_manual_signal(SignalType::Trap, SignalEvent::Yield);
    }

    void terminate_current_task() override
    {
        // 1. 主动触发一个 Trap 类型的信号，ID 约定为 Terminate
        // 这会让内核进入 on_signal_received 逻辑
        _dispatcher->trigger_manual_signal(SignalType::Trap, SignalEvent::Terminate);
        // 彻底终结当前 Windows 线程
        ExitThread(0);
    }
};