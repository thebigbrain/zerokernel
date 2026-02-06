#pragma once

#include <common/TaskTypes.hpp>
#include <kernel/ITaskContext.hpp>

class MockTaskContext : public ITaskContext
{
private:
    friend class ContextInspector;

private:
    void (*_entry)(void *, void *) = nullptr;
    uintptr_t _args[4] = {0};
    void *_stack_pointer = nullptr;

    bool _has_executed = false;
    uint32_t _jump_count = 0;

public:
    // 针对 Mock 环境的额外接口
    bool has_executed() const { return _has_executed; }
    uint32_t get_jump_count() const { return _jump_count; }

    size_t get_context_size() const override { return 0; }

    void transit_to(ITaskContext *target) override
    {
    }

    void setup_flow(void (*entry)(void *, void *), void *stack_top) override
    {
        _entry = entry;
        _stack_pointer = stack_top;
    }

    void load_argument(size_t index, uintptr_t value) override
    {
        if (index < 4)
            _args[index] = value;
    }

    void *get_stack_pointer() const override
    {
        return _stack_pointer;
    }
};