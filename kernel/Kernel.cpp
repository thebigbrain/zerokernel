#include <cassert>
#include "Kernel.hpp"
#include "ITaskManager.hpp"
#include "KernelProxy.hpp"
#include "KernelCallback.hpp"
#include <common/BootInfo.hpp>

// 这个宏的作用是：自动生成一个静态的“跳板”并打包成 KernelCallback 对象
#define BIND_KERNEL_CB(Class, Func, ObjPtr)                                      \
    KernelCallback(                                                              \
        [](const Message &m, void *ctx) { static_cast<Class *>(ctx)->Func(m); }, \
        static_cast<void *>(ObjPtr))

// 使用示例（如果你的编译器支持这种 Lambda 到指针的转换）
// _bus->subscribe(TYPE, BIND_KERNEL_CB(handle_load_task, this));

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

    void *bus_mem = _factory->allocate_raw(sizeof(MessageBus));
    this->_bus = new (bus_mem) MessageBus(_factory);

    _bus->subscribe(MessageType::SYS_LOAD_TASK, BIND_KERNEL_CB(Kernel, handle_load_task, this));
    _bus->subscribe(MessageType::EVENT_PRINT, BIND_KERNEL_CB(Kernel, handle_event_print, this));

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
        _bus->dispatch_messages();

        // 执行调度逻辑（挑选有消息或就绪的任务进行 transit）
        yield();
    }
}

Task *Kernel::create_task_skeleton(void (*entry_point)())
{
    if (_task_count >= 64)
        return nullptr;

    // 1. 创建架构相关的上下文 (Context)
    size_t ctx_size = _cpu->get_context_size();
    void *ctx_mem = _factory->allocate_raw(ctx_size);
    ITaskContext *ctx = _cpu->create_context_at(ctx_mem);

    assert(ctx != nullptr);

    // 2. 创建 Task 对象 (统一使用 _task_count 作为内部 ID)
    uint32_t tid = _task_count;
    Task *t = _factory->create<Task>(tid, ctx);

    // 3. 分配并初始化栈
    t->init_stack(_factory, 64 * 1024);

    // 4. 准备入口地址和栈帧
    t->prepare(entry_point, t->get_stack_top());

    return t;
}

// 场景 1：创建普通内核任务（无参数）
Task *Kernel::spawn_task(void (*entry_point)())
{
    Task *t = create_task_skeleton(entry_point);
    if (!t)
        return nullptr;

    // 注册并递增计数器
    _tasks[_task_count++] = t;
    _ready_queue.push(t); // 确保 yield 能看到它

    return t;
}

// 场景 2：创建固定任务（如 RootTask，带 Proxy 和 Config 参数）
Task *Kernel::spawn_fixed_task(void *task_entry, void *config)
{
    // 转换为函数指针并创建骨架
    Task *t = create_task_skeleton((void (*)())task_entry);
    if (!t)
        return nullptr;

    // 1. 特有的逻辑：实例化 Proxy
    KernelProxy *proxy = _factory->create<KernelProxy>(_bus, this);

    // 2. 特有的逻辑：填充参数 (ABI 相关)
    t->set_parameter(0, (uintptr_t)proxy);
    t->set_parameter(1, (uintptr_t)config);

    // 3. 注册、递增计数器并入队
    _tasks[_task_count++] = t;
    _ready_queue.push(t);

    return t;
}

void Kernel::yield()
{
    // 1. 基础安全检查：如果没有任务可跑，直接回退
    if (_ready_queue.empty())
    {
        return;
    }

    // 2. 获取下一个要跑的任务
    Task *next = _ready_queue.front();
    _ready_queue.pop();

    // 3. 记录旧任务（可能是 nullptr，代表内核启动前的初始状态）
    Task *prev = _current;

    // 4. 更新当前指向
    _current = next;

    // 5. 判定切换逻辑
    if (prev == nullptr)
    {
        // --- 场景 A：内核启动后的第一次调度 ---
        // 此时不需要保存旧上下文（因为没有旧任务在跑）
        // 直接“跳入”新任务的上下文执行
        auto ctx = next->get_context();
        _cpu->execute(ctx);
    }
    else
    {
        // --- 场景 B：正常任务切换 ---
        // 检查是否切换到了自己（队列里只有自己时）
        if (prev == next)
        {
            _ready_queue.push(next); // 放回队列
            return;
        }

        // 将旧任务放回就绪队列尾部
        _ready_queue.push(prev);

        // 执行架构相关的上下文切换
        // 保存 prev 的状态，恢复 next 的状态
        _cpu->transit(prev->get_context(), next->get_context());
    }
}

Task *Kernel::get_ready_task(int index)
{
    if (index < _task_count)
    {
        return _tasks[index];
    }
    return nullptr; // 或者返回 Idle 任务
}
