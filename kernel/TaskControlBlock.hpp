#pragma once

#include "ITaskControlBlock.hpp"

/**
 * TaskControlBlock: ITaskControlBlock 的标准实现
 * 负责持有任务运行所需的元数据、上下文句柄以及通信信箱
 */
class TaskControlBlock : public ITaskControlBlock
{
private:
    uint32_t _id;
    TaskState _state;
    ITaskContext *_context; // 体系结构相关的上下文句柄
    Mailbox _mailbox;       // 每个任务自带的信箱

public:
    /**
     * 构造函数：由 ObjectFactory 自动调用
     * @param id 任务 ID
     * @param context 已分配并初始化的体系结构上下文
     */
    TaskControlBlock(uint32_t id, ITaskContext *context)
        : _id(id),
          _state(TaskState::CREATED),
          _context(context),
          _mailbox(32) // 默认容量 32 条消息
    {
    }

    virtual ~TaskControlBlock()
    {
        // 注意：_context 的内存通常由 ObjectFactory 管理并统一回收
    }

    // --- 接口实现 ---

    uint32_t get_id() const override
    {
        return _id;
    }

    void set_state(TaskState state) override
    {
        _state = state;
    }

    TaskState get_state() const override
    {
        return _state;
    }

    ITaskContext *get_context() override
    {
        return _context;
    }

    Mailbox *get_mailbox() override
    {
        return &_mailbox;
    }
};