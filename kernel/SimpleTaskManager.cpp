#include "SimpleTaskManager.hpp"
#include "TaskControlBlock.hpp"
#include <cstdio>

void set_active_task_manager(ITaskManager *mgr);

SimpleTaskManager::SimpleTaskManager(ObjectFactory *f, ICPUEngine *cpu, BootInfo *info)
    : _factory(f), _cpu(cpu), _boot_info(info), _current(nullptr), _task_count(0)
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

ITaskControlBlock *SimpleTaskManager::spawn_task(void *entry_point)
{
    if (_task_count >= 64)
        return nullptr;

    uint32_t id = _task_count++;
    ITaskControlBlock *t = create_tcb_internal((void (*)())entry_point, id);

    if (t)
    {
        _tasks[id] = t;
        _ready_queue.push(t);
        t->set_state(TaskState::READY);
    }

    return t;
}

void SimpleTaskManager::spawn_fixed_task(void *entry, void *config, void *proxy)
{
    // 复用 internal 创建逻辑
    ITaskControlBlock *t = create_tcb_internal((void (*)())entry, _task_count++);

    if (t)
    {
        _tasks[t->get_id()] = t;

        // 注入 ABI 参数：第一个参数通常是代理，第二个是配置
        t->get_context()->set_parameter(0, (uintptr_t)proxy);
        t->get_context()->set_parameter(1, (uintptr_t)config);

        t->set_state(TaskState::READY);
        _ready_queue.push(t);
    }
}

void SimpleTaskManager::spawn_task_from_message(const Message &msg)
{
    void (*entry)() = (void (*)())msg.payload[0];
    uint32_t task_id = (uint32_t)msg.payload[1];

    if (task_id >= 64)
        return;

    ITaskControlBlock *t = this->create_tcb_internal(entry, task_id);

    if (t)
    {
        _tasks[task_id] = t;
        t->set_state(TaskState::READY);
        _ready_queue.push(t);
        printf("[TaskManager] Task %d loaded via Message.\n", task_id);
    }
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
    if (_ready_queue.empty())
        return;

    // 选出下一个
    ITaskControlBlock *next = pick_next_ready_task();
    ITaskControlBlock *prev = _current;

    _current = next;

    if (prev == nullptr)
    {
        // 第一次启动
        _cpu->switch_to(next->get_context());
    }
    else if (prev != next)
    {
        // 发生切换：将旧任务放回队列
        make_task_ready(prev);
        _cpu->transit(prev->get_context(), next->get_context());
    }
    else
    {
        // 只有自己在跑
        make_task_ready(next);
    }
}

void SimpleTaskManager::terminate_current_task()
{
    if (!_current)
        return;

    printf("[TaskManager] Task %d terminated.\n", _current->get_id());

    _current->set_state(TaskState::DEAD);

    // 清理追踪记录，防止 get_task 拿到已死的任务
    _tasks[_current->get_id()] = nullptr;

    _current = nullptr;
    this->yield_current_task();
}

// -------------------------------------------------------------------------
// 基础设施
// -------------------------------------------------------------------------

BootInfo *SimpleTaskManager::get_boot_info()
{
    // 这里应当由 Kernel 在 bootstrap 阶段通过某种方式传入
    // 暂时假设有某种手段获取全局 BootInfo 或在构造时传入
    return _boot_info;
}

ITaskControlBlock *SimpleTaskManager::create_tcb_internal(void (*entry)(), uint32_t id)
{
    // 1. 利用 CPU 引擎创建架构相关的上下文
    void *ctx_mem = _factory->allocate_raw(_cpu->get_context_size());
    ITaskContext *ctx = _cpu->create_context_at(ctx_mem);

    // 2. 利用工厂创建具体的 TCB 实例
    ITaskControlBlock *t = _factory->create<TaskControlBlock>(id, ctx);

    // 3. 分配栈空间（16KB）
    void *stack = _factory->allocate_raw(16384);

    // 4. 初始化上下文：设定入口、栈顶、以及任务退出时的跳转路由
    ctx->prepare(entry, (uint8_t *)stack + 16384, task_exit_router);

    return t;
}