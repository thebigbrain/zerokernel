#pragma once

#include <atomic>
#include <iostream>
#include "kernel/ISignal.hpp"

class MockSignalDispatcher : public ISignalGate
{
private:
    ISignalListener *m_listener = nullptr;
    std::atomic<bool> m_active{false};

public:
    // 绑定内核监听器：内核处理函数（如 TrapHandler）会通过这个接口注册进来
    void bind_listener(ISignalListener *listener) override
    {
        m_listener = listener;
        std::cout << "[Mock Dispatcher] Listener bound." << std::endl;
    }

    // 激活：模拟开启 CPU 全局中断
    void activate() override
    {
        m_active = true;
        std::cout << "[Mock Dispatcher] Signals Activated (Interrupts On)." << std::endl;
    }

    // 关闭：模拟进入临界区（CLI 指令）
    void deactivate() override
    {
        m_active = false;
        std::cout << "[Mock Dispatcher] Signals Deactivated (Interrupts Off)." << std::endl;
    }

    // --- 模拟器特有方法：手动触发一个信号 ---
    void trigger_mock_signal(SignalEvent signal_id)
    {
        if (m_active && m_listener)
        {
            // 模拟异步中断的到来
            m_listener->on_signal_received(SignalPacket{SignalType::Interrupt, signal_id, nullptr});
        }
        else if (!m_active)
        {
            std::cout << "[Mock Dispatcher] Signal ignored (Deactivated)." << std::endl;
        }
    }

    bool is_active() const { return m_active; }
};
