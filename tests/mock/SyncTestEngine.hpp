#include "kernel/IExecutionEngine.hpp"
#include "kernel/ITaskManager.hpp"
#include "common/IUserRuntime.hpp"

class SyncTestEngine : public IExecutionEngine
{
private:
    ITaskManager *_tm;
    IUserRuntime *_rt;

public:
    // 注入必要的依赖，以便引擎能找到任务和运行环境
    SyncTestEngine(ITaskManager *tm, IUserRuntime *rt)
        : _tm(tm), _rt(rt) {}

    void start() override
    {
        ITaskControlBlock *current = _tm->get_current_task();
        if (!current)
            return;

        // 领域模型上的直接依赖：直接获取执行信息
        const TaskExecutionInfo &info = current->get_execution_info();
        auto entry = (void (*)(void *, void *))info.entry;

        entry(_rt, info.config);
    }

    // 同步引擎在测试中通常不需要复杂的抢占调度
    void schedule_task(ITaskControlBlock *task) override
    {
        // 如果是异步消息触发了新任务，可以在这里立刻同步调用它
    }
};