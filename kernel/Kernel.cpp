#include "Kernel.hpp"
#include "ITaskManager.hpp"
#include "KernelProxy.hpp"
#include <common/BootInfo.hpp>

static ITaskManager *g_manager_instance = nullptr;
Kernel *Kernel::instance = nullptr;

// 提供给外部注入实例的方法
void set_task_manager_instance(ITaskManager *mgr)
{
    g_manager_instance = mgr;
}

// 汇编 ret 指令跳转到这里
extern "C" void task_exit_router()
{
    if (g_manager_instance)
    {
        g_manager_instance->terminate_current_task();
    }
    // 如果没有管理器，系统可能需要 halt
}

void Kernel::bootstrap(BootInfo *info)
{
    _boot_info = info;

    set_task_manager_instance(this);
    _bus->subscribe(MessageType::SYS_LOAD_TASK, [this](const Message &msg)
                    { this->handle_load_task(msg); });

    _bus->subscribe(MessageType::EVENT_PRINT, [this](const Message &msg)
                    { this->handle_event_print(msg); });

    // 直接拉起 RootTask
    this->spawn_fixed_task((void *)_boot_info->root_task_entry, _boot_info->config_ptr);
    this->spawn_fixed_task((void *)_boot_info->idle_task_entry, nullptr);

    this->run_loop();
}

void Kernel::handle_load_task(const Message &msg)
{
    void (*entry)() = (void (*)())msg.payload[0];
    uint32_t id = (uint32_t)msg.payload[1];

    // 从 ObjectFactory 申请内存
    void *ctx_mem = _factory->allocate_raw(_cpu->get_context_size());
    ITaskContext *ctx = _cpu->create_context_at(ctx_mem);
    void *stack_top = _factory->allocate_raw(16384);

    // 创建任务对象
    Task *t = _factory->create<Task>(id, ctx);
    t->init_stack(_factory, 16 * 1024);
    t->prepare(entry, t->get_stack_top());

    _tasks[id] = t;
    printf("[Kernel] Event Handled: Task %d instantiated at %p\n", id, entry);
}

void Kernel::handle_event_print(const Message &msg)
{
    // 解码 payload 里的字符
    char buf[9] = {0};
    memcpy(buf, &msg.payload[0], 8);

    // 利用模拟器后门打印到 Windows 控制台
    printf("[Kernel Log] Received from RootTask: %s\n", buf);
}

void Kernel::run_loop()
{
    while (true)
    {
        // 处理总线上的所有事件（分发给各订阅者，包括内核自己）
        _bus->process_and_route();

        // 执行调度逻辑（挑选有消息或就绪的任务进行 transit）
        yield();
    }
}

Task *Kernel::spawn_task(void (*entry_point)())
{
    if (_task_count >= 64)
        return nullptr;

    // 1. 架构无关地创建 Context
    size_t ctx_size = _cpu->get_context_size();
    void *ctx_mem = _factory->allocate_raw(ctx_size);
    ITaskContext *ctx = _cpu->create_context_at(ctx_mem);

    // 2. 创建 Task 对象
    Task *newTask = _factory->create<Task>(_task_count, ctx);
    newTask->init_stack(_factory, 64 * 1024);

    // 4. 初始化（在之前的讨论中，这里会填入 entry 和 exit_gate）
    newTask->prepare(entry_point, newTask->get_stack_top());

    // 5. 注册到就绪数组
    _tasks[_task_count++] = newTask;

    return newTask;
}

Task *Kernel::spawn_fixed_task(
    void *task_entry, void *config)
{
    // 1. 生成唯一 ID (假设 RootTask 始终为 1)
    uint32_t tid = this->generate_unique_id();

    // 2. 在内核受控内存中实例化代理
    // 注意：proxy 的生命周期由内核通过 ObjectFactory 管理
    KernelProxy *proxy = _factory->create<KernelProxy>(_bus, this);

    // 获取架构相关的 Context 处理器
    void *ctx_mem = _factory->allocate_raw(_cpu->get_context_size());
    ITaskContext *ctx = _cpu->create_context_at(ctx_mem);

    // 5. 封装为 Task 对象
    Task *t = _factory->create<Task>(tid, ctx);
    t->init_stack(_factory, 64 * 1024); // 封装了 allocate_raw 和指针运算

    // 设置任务入口并平衡栈帧（压入 task_exit_gate）
    t->prepare((void (*)())task_entry, t->get_stack_top());
    t->set_parameter(0, (uintptr_t)proxy);
    t->set_parameter(1, (uintptr_t)config);

    // 6. 加入就绪队列
    this->_tasks[tid] = t;
    this->_ready_queue.push(t);

    return t;
}

void Kernel::yield()
{
    if (_task_count < 2)
        return;

    int old_index = _current_index;
    _current_index = (_current_index + 1) % _task_count;

    Task *prev = _tasks[old_index];
    Task *next = _idle_task;

    if (!_ready_queue.empty())
    {
        next = _ready_queue.front();
        _ready_queue.pop();
    }

    _cpu->transit(prev->get_context(), next->get_context());
}

Task *Kernel::get_ready_task(int index)
{
    if (index < _task_count)
    {
        return _tasks[index];
    }
    return nullptr; // 或者返回 Idle 任务
}
