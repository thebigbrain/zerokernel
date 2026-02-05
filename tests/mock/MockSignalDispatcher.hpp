#pragma once

#include <kernel/ISignal.hpp>

/**
 * @file MockSignalDispatcher.cpp
 * 职责：手动模拟信号分发，用于测试 Kernel 的逻辑
 */
class MockSignalDispatcher : public ISignalDispatcher
{
private:
    ISignalListener *_listener = nullptr;
    bool _active = false;

public:
    void bind_listener(ISignalListener *listener) override
    {
        _listener = listener;
    }

    void activate() override
    {
        _active = true;
    }

    // 这是一个 Mock 专有的方法，由你的测试脚本调用
    void trigger_mock_signal(SignalType type, uint32_t vector, ISignalContext *frame)
    {
        if (_active && _listener)
        {
            SignalPacket packet{type, vector, frame};
            // 模拟硬件中断：直接调用 Kernel 的监听接口
            _listener->on_signal_received(packet);
        }
    }
};