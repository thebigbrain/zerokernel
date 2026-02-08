#pragma once

#include "TaskScheduler.hpp"
#include "ISignal.hpp"
#include "SignalType.hpp"
#include <common/diagnostics.hpp>

struct YieldHandler
{
    static void handle(TaskScheduler &scheduler, SignalPacket &packet)
    {
        K_DEBUG("Dispatcher: Handling Yield Signal (ID: %d)", packet.event_id);

        // 核心逻辑委派给调度器
        scheduler.yield_current();
    }
};

class SignalDispatcher
{
public:
    SignalDispatcher(TaskScheduler &sched) : _sched(sched) {}

    void dispatch(SignalPacket &packet)
    {
        switch (packet.type)
        {
        case SignalType::Yield:
            YieldHandler::handle(_sched, packet);
            break;
        case SignalType::Interrupt:
            // handle_keyboard();
            break;
            // 其他信号...
        }
    }

private:
    TaskScheduler &_sched;
};
