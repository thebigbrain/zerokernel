#pragma once

#include "ITaskContext.hpp"
#include "ITaskManager.hpp"
#include "common/Message.hpp"
#include "ITask.hpp"
#include "ObjectFactory.hpp"

#include <cstdint>
#include <queue>

class Task : public ITask
{
private:
    uint32_t _id;
    ITaskContext *_context;

    void *_stack_mem; // 记录分配的起始地址，用于释放
    size_t _stack_size;

    std::queue<Message> mailbox;

public:
    Task(uint32_t id, ITaskContext *ctx) : _id(id), _context(ctx) {}

    uint32_t get_id() const { return _id; }

    void on_message(const Message &msg)
    {
    }

    void push_message(const Message &msg) { mailbox.push(msg); }
    bool has_message() { return !mailbox.empty(); }
    Message pop_message()
    {
        Message m = mailbox.front();
        mailbox.pop();
        return m;
    }

    void init_stack(ObjectFactory *factory, size_t size)
    {
        _stack_size = size;
        _stack_mem = factory->allocate_raw(size);
    }

    void *get_stack_top() const
    {
        return (uint8_t *)_stack_mem + _stack_size;
    }

    void prepare(void (*entry)(), void *stack_top)
    {
        // 确保 stack_top 是 8 字节对齐的（x64 必须）
        uintptr_t top = (uintptr_t)stack_top;
        top &= ~0x7ULL;

        uintptr_t *stack = (uintptr_t *)top;

        // 1. 压入退出路由器 (在 top 之下写入)
        // 此时写入的是栈的最顶端槽位
        stack[-1] = (uintptr_t)task_exit_router;

        // 2. 将调整后的栈顶交给 Context
        // 下一次压栈会从 stack[-2] 开始
        void *adjusted_stack = (void *)&stack[-1];
        _context->prepare(entry, adjusted_stack);
    }

    void set_parameter(int index, uintptr_t value)
    {
        _context->set_parameter(index, value);
    }

    ITaskContext *get_context() const { return _context; }
};