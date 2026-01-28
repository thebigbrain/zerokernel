#pragma once

extern "C" void task_exit_router();

/**
 * 任务管理器接口：定义了任务退出时的收割行为
 */
class ITaskManager
{
public:
    virtual ~ITaskManager() = default;

    // 供 Task 退出时调用的抽象接口
    virtual void terminate_current_task() = 0;
};
