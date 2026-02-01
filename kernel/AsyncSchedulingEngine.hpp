#include "IExecutionEngine.hpp"
#include "ITaskManager.hpp"
#include "ICPUEngine.hpp"

class AsyncSchedulingEngine : public IExecutionEngine
{
private:
    ITaskManager *_tm;
    ICPUEngine *_cpu;

public:
    AsyncSchedulingEngine(ITaskManager *tm, ICPUEngine *cpu) : _tm(tm), _cpu(cpu)
    {
    }

    void start(void *entry, void *arg, IUserRuntime *rt) override;

    void schedule_task(ITaskControlBlock *task) override {}

private:
    void enter_hardware_run_loop();
};