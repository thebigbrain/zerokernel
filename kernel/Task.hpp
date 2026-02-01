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
        // 只负责对齐最初的栈顶（虽然底层也会做，但这里做一次更安全）
        uintptr_t top = (uintptr_t)stack_top;
        top &= ~0xFULL; // 调整为 16 字节对齐

        // 将入口点和退出路由器都传给 Context
        // 这样 Context 知道如何根据 ABI（如 x64, ARM）摆放它们
        _context->prepare(entry, (void *)top, task_exit_router);
    }

    void set_parameter(int index, uintptr_t value)
    {
        _context->set_parameter(index, value);
    }

    ITaskContext *get_context() const { return _context; }
};