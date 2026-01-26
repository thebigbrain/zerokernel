#pragma once
#include "ITaskContext.hpp"

class Task
{
private:
    uint32_t _id;
    ITaskContext *_context;

public:
    Task(uint32_t id, ITaskContext *ctx) : _id(id), _context(ctx) {}

    ITaskContext *get_context() const { return _context; }
    uint32_t get_id() const { return _id; }

    void prepare(void (*entry)(), void *stack_top)
    {
        _context->prepare(entry, stack_top);
    }
};