#pragma once

#include <kernel/ITaskContext.hpp>

class MockTaskContext : public ITaskContext
{
private:
    void (*_entry)() = nullptr;
    uintptr_t _args[4] = {0};
    void *_stack_pointer = nullptr;

public:
    size_t get_context_size() const override { return 0; }

    // 核心逻辑：jump_to 直接触发函数调用
    void jump_to() override
    {
        if (_entry)
        {
            // 模拟跳转：将 entry 转换为带参数的签名并执行
            typedef void (*TaskEntryFunc)(void *, void *);
            auto func = reinterpret_cast<TaskEntryFunc>(_entry);
            func((void *)_args[0], (void *)_args[1]);
        }
    }

    void transit_to(ITaskContext *target) override
    {
        // 同步测试中，transit_to 等同于让目标 jump_to
        target->jump_to();
    }

    void setup_flow(void (*entry)(), void *stack_top, void (*exit_router)()) override
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