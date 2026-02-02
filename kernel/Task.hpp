#pragma once

#include "ITaskContext.hpp"
#include "ITaskManager.hpp"
#include "common/Message.hpp"
#include "ITaskControlBlock.hpp"
#include "ObjectFactory.hpp"
#include "Mailbox.hpp"

#include <cstdint>

class Task
{
private:
    uint32_t _id;
    ITaskContext *_context;
    ITaskControlBlock *_tcb;

    void *_stack_mem; // 记录分配的起始地址，用于释放
    size_t _stack_size;

    Mailbox _mailbox;

public:
    Task(uint32_t id, ITaskContext *ctx) : _id(id), _context(ctx) {}

    uint32_t get_id() const { return _id; }

    Mailbox &get_mailbox() { return _mailbox; }

    void init_stack(ObjectFactory *factory, size_t size)
    {
        _stack_size = size;
        _stack_mem = factory->allocate_raw(size);
    }

    ITaskContext *get_context() const { return _context; }
};