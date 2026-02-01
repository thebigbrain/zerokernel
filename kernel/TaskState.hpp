#pragma once

#include <cstdint>

/**
 * 任务生命周期状态
 * 用于 ITaskManager 调度决策
 */
enum class TaskState : uint32_t
{
    CREATED,  // 已分配资源，尚未开始执行
    READY,    // 进入就绪队列，等待 CPU 时间片
    RUNNING,  // 正在当前 CPU 核心上执行
    BLOCKED,  // 等待某个事件（如 IPC 消息、信号量、IO）
    SLEEPING, // 被主动挂起或定时睡眠
    DEAD      // 已执行完毕，等待 ITaskManager 回收资源
};