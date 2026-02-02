#include "SimpleTaskManager.hpp"
#include "TaskControlBlock.hpp"
#include <cstdio>

void set_active_task_manager(ITaskManager *mgr);

SimpleTaskManager::SimpleTaskManager(ObjectFactory *f, ITaskContextFactory *tcf, ITaskControlBlockFactory *tf)
    : _obj_factory(f), _context_factory(tcf), _tcb_factory(tf), _current_task(nullptr), _task_count(0)
{
    // 初始化任务追踪数组
    for (int i = 0; i < 64; ++i)
    {
        _tasks[i] = nullptr;
    }
    set_active_task_manager(this);
}

SimpleTaskManager::~SimpleTaskManager()
{
    set_active_task_manager(nullptr);
}

// -------------------------------------------------------------------------
// 生命周期管理
// -------------------------------------------------------------------------

// 核心：所有创建路径最终都汇聚到这里
ITaskControlBlock *SimpleTaskManager::create_and_register(TaskEntry entry, void *arg, TaskPriority prio)
{
    if (_task_count >= MAX_TASKS)
        return nullptr;

    TaskExecutionInfo exec;
    exec.entry = entry;
    exec.config = arg;
    TaskResourceConfig res;
    res.priority = prio;
    res.stack_size = 16384;

    auto *ctx = _context_factory->create_context();
    auto *tcb = _tcb_factory->create_tcb(_task_count, ctx, exec, res);

    if (tcb)
        this->register_task(tcb);
    return tcb;
}

// 现在 spawn_task 只需要一行逻辑
ITaskControlBlock *SimpleTaskManager::spawn_task(void *entry, TaskPriority prio, void *config)
{
    return create_and_register(reinterpret_cast<TaskEntry>(entry), config, prio);
}

// 消息版本也变得非常清晰
void SimpleTaskManager::spawn_task_from_message(const Message &msg)
{
    auto *tcb = create_and_register(
        reinterpret_cast<TaskEntry>(msg.payload[0]), // entry
        reinterpret_cast<void *>(msg.payload[1]),    // arg/config
        static_cast<TaskPriority>(msg.payload[2])    // prio
    );
    if (tcb)
        printf("[TaskManager] Task %d loaded via Message.\n", tcb->get_id());
}

// -------------------------------------------------------------------------
// 调度逻辑
// -------------------------------------------------------------------------

ITaskControlBlock *SimpleTaskManager::pick_next_ready_task()
{
    if (_ready_queue.empty())
        return nullptr;

    ITaskControlBlock *next = _ready_queue.front();
    _ready_queue.pop();

    next->set_state(TaskState::RUNNING);
    return next;
}

void SimpleTaskManager::make_task_ready(ITaskControlBlock *task)
{
    if (!task)
        return;
    task->set_state(TaskState::READY);
    _ready_queue.push(task);
}

void SimpleTaskManager::yield_current_task()
{
    // 1. 决策逻辑：选出下一个任务
    ITaskControlBlock *next = pick_next_ready_task();
    if (!next)
        return;

    ITaskControlBlock *prev = _current_task;
    _current_task = next;

    // 2. 状态流转与执行切换
    if (prev == nullptr)
    {
        // 场景 A：系统首次启动，没有“前任”需要保存
        next->get_context()->jump_to();
    }
    else if (prev != next)
    {
        // 场景 B：真正的任务切换
        make_task_ready(prev); // 将旧任务放回就绪列表

        // 语义化切换：由当前上下文发起向目标上下文的过渡
        prev->get_context()->transit_to(next->get_context());
    }
    else
    {
        // 场景 C：只有自己在跑，不需要切换硬件状态，但需要维持就绪态
        make_task_ready(next);
    }
}

void SimpleTaskManager::terminate_current_task()
{
    if (!_current_task)
        return;

    auto tid = _current_task->get_id();

    printf("[TaskManager] Task %d terminated.\n", tid);

    _current_task->set_state(TaskState::DEAD);

    // 清理追踪记录，防止 get_task 拿到已死的任务
    _tasks[tid] = nullptr;

    _current_task = nullptr;
    this->yield_current_task();
}

void SimpleTaskManager::register_task(ITaskControlBlock *tcb)
{
    if (!tcb)
        return;

    uint32_t id = tcb->get_id();

    // 再次确认 ID 是否合法（防止外部非法调用）
    if (id < MAX_TASKS)
    {
        _tasks[id] = tcb;
        _task_count++; // 只有在这里才真正增加任务总数

        // 通常新创建的任务默认为就绪态
        make_task_ready(tcb);
    }
}

ITaskControlBlock *SimpleTaskManager::get_task(uint32_t task_id)
{
    if (task_id >= MAX_TASKS)
        return nullptr;
    return _tasks[task_id];
}