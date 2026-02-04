#pragma once

#include <cstdint>

enum class TaskPriority : uint8_t
{
    IDLE = 0,     // 空闲任务，最低优先级
    LOW = 1,      // 后台任务
    NORMAL = 2,   // 普通应用任务
    HIGH = 3,     // 关键服务任务
    REALTIME = 4, // 实时/硬实时任务
    ROOT = 5      // 内核启动任务 (RootTask) 的特权优先级
};

enum class TaskState : uint32_t
{
    CREATED,  // 已分配资源，尚未开始执行
    READY,    // 进入就绪队列，等待 CPU 时间片
    RUNNING,  // 正在当前 CPU 核心上执行
    BLOCKED,  // 等待某个事件（如 IPC 消息、信号量、IO）
    SLEEPING, // 被主动挂起或定时睡眠
    DEAD      // 已执行完毕，等待 ITaskLifecycle 回收资源
};

typedef void (*TaskEntry)(void *, void *);

class IUserRuntime;

struct TaskExecutionInfo
{
    TaskEntry entry; // 明确的函数指针类型
    IUserRuntime *runtime;
    void *config; // 任务私有配置
};

/**
 * TaskResourceConfig: 定义任务的资源约束
 */

class KStackBuffer;
struct TaskResourceConfig
{
    TaskPriority priority;
    KStackBuffer *stack; // 不再是裸指针，而是受管对象

    TaskResourceConfig()
        : priority(TaskPriority::NORMAL), stack(nullptr) {}

    TaskResourceConfig(TaskPriority priority, KStackBuffer *stack)
        : priority(priority), stack(stack) {}
};

/**
 * TaskSpawnParams: 综合创建参数
 * 它是业务层发往内核的“完整蓝图”
 */
struct TaskSpawnParams
{
    TaskExecutionInfo exec_info;   // 逻辑：代码在哪里，用什么运行时
    TaskResourceConfig res_config; // 资源：给多少内存，优先级多高
};