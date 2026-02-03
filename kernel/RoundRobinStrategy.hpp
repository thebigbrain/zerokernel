#pragma once

#include "ISchedulingStrategy.hpp"
#include "IObjectBuilder.hpp"
#include "ITaskControlBlock.hpp"
#include "KList.hpp"

class RoundRobinStrategy : public ISchedulingStrategy
{
private:
    KList<ITaskControlBlock *> _ready_queue;

public:
    RoundRobinStrategy(IObjectBuilder *builder)
        : _ready_queue(builder)
    {
    }

    void make_task_ready(ITaskControlBlock *tcb) override
    {
        if (tcb && !tcb->is_queued())
        {
            _ready_queue.push_back(tcb);
            tcb->set_queued(true); // 必须标记已在队列中
        }
    }

    ITaskControlBlock *pick_next_ready_task() override
    {
        if (_ready_queue.empty())
            return nullptr;

        auto *next = _ready_queue.front();
        _ready_queue.pop_front();

        // 出队后必须清除标记，否则它无法再次通过 make_task_ready 入队
        if (next)
        {
            next->set_queued(false);
        }

        return next;
    }

    void remove_task(ITaskControlBlock *tcb) override
    {
        if (tcb && tcb->is_queued())
        {
            _ready_queue.remove_match(
                [tcb](ITaskControlBlock *item)
                { return item == tcb; });

            tcb->set_queued(false);
        }
    }
};