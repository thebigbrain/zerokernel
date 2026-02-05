#pragma once

#include <windows.h>
#include <iostream>

#include <kernel/ISignal.hpp>
#include <kernel/SignalType.hpp>
#include "Win32SignalContext.hpp"

class Win32SignalDispatcher : public ISignalDispatcher
{
private:
    ISignalListener *_listener = nullptr;
    HANDLE _target_thread; // 被模拟的任务线程（如 RootTask 所在的线程）
    bool _active = false;

public:
    // 注入当前正在运行的任务线程句柄
    void set_target_thread(HANDLE thread_handle)
    {
        _target_thread = thread_handle;
    }

    void bind_listener(ISignalListener *listener) override
    {
        _listener = listener;
    }

    void activate() override
    {
        _active = true;
        std::cout << "[Win32 Dispatcher] Signals Activated." << std::endl;
    }

    void deactivate() override
    {
        _active = false;
        std::cout << "[Win32 Dispatcher] Signals Deactivated." << std::endl;
    }

    /**
     * @brief 模拟硬件指令触发信号
     * 由 Win32SchedulingControl 调用
     */
    void trigger_manual_signal(SignalType type, SignalEvent event_id)
    {
        HANDLE hThread = GetCurrentThread();

        // 1. 捕获当前任务线程的临终/瞬时现场
        CONTEXT win_ctx;
        win_ctx.ContextFlags = CONTEXT_FULL;
        GetThreadContext(hThread, &win_ctx);

        // 2. 包装并分发给内核监听者
        Win32SignalContext sig_ctx(win_ctx);
        SignalPacket packet{type, event_id, &sig_ctx};

        // _listener 是通过 bind_listener 绑定的 Kernel
        _listener->on_signal_received(packet);
    }

    /**
     * @brief 模拟物理中断触发
     * 这个方法可以由一个专门的定时器线程调用，模拟 Tick 中断
     */
    void trigger_interrupt(SignalEvent vector)
    {
        if (!_active || !_listener)
            return;

        // 1. 强行挂起目标线程（模拟硬件中断打断 CPU）
        SuspendThread(_target_thread);

        // 2. 获取该线程被截获时的瞬时现场
        CONTEXT win_context;
        win_context.ContextFlags = CONTEXT_FULL;
        if (GetThreadContext(_target_thread, &win_context))
        {

            // 3. 包装成你的 ISignalContext (Win32 特化版)
            Win32SignalContext signal_ctx(win_context);

            // 4. 构造包并分发给 Kernel
            SignalPacket packet{SignalType::Interrupt, vector, &signal_ctx};
            _listener->on_signal_received(packet);

            // 5. 如果内核决定继续运行，则恢复线程
            // 注意：如果内核决定切换任务，这里逻辑会更复杂，涉及线程切换
            ResumeThread(_target_thread);
        }
    }
};