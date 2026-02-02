#include "IExecutionEngine.hpp"
#include "ITaskManager.hpp"

class AsyncSchedulingEngine : public IExecutionEngine
{
private:
    ITaskManager *_tm;

public:
    AsyncSchedulingEngine(ITaskManager *tm) : _tm(tm)
    {
    }

    void start() override
    {
        ITaskControlBlock *first = _tm->pick_next_ready_task();
        if (first)
        {
            _tm->register_task(first);
            first->get_context()->jump_to(); // 开启万物
        }
    }

    void schedule_task(ITaskControlBlock *tcb) override
    {
        auto current_task = _tm->get_current_task();
        auto priority = tcb->get_resource_config().priority;
        // 这里体现策略：如果是抢占式，就在这里对比优先级
        if (priority > current_task->get_resource_config().priority)
        {
            _tm->yield_current_task(); // 触发切换
        }
    }

private:
    void enter_hardware_run_loop();
};