#pragma once

#include <common/TaskTypes.hpp>
#include <kernel/ITaskContext.hpp>

class MockTaskContext : public ITaskContext
{
private:
    friend class ContextInspector;

private:
    void (*_entry)() = nullptr;
    uintptr_t _args[4] = {0};
    void *_stack_pointer = nullptr;

    bool _has_executed = false;
    uint32_t _jump_count = 0;

public:
    void jump_to() override
    {
        // 1. 打印即将跳转的现场
        std::cout << "[Jump] Target: " << (void *)_entry << " Arg0: " << (void *)_args[0] << std::endl;

        if (!_entry)
        {
            std::cerr << "[Panic] Attempting to jump to NULL!" << std::endl;
            return;
        }

        // 2. 这里的强转必须与你 RootTask 的签名完全一致
        typedef void (*TaskEntryFunc)(void *, void *);
        auto func = reinterpret_cast<TaskEntryFunc>(_entry);

        // 3. 记录跳转前的一刻
        _has_executed = true;
        _jump_count++;

        // 如果在这里崩溃，说明是 func 内部逻辑的问题
        func((void *)_args[0], (void *)_args[1]);

        std::cout << "[Jump] Task returned safely." << std::endl;
    }

    // 针对 Mock 环境的额外接口
    bool has_executed() const { return _has_executed; }
    uint32_t get_jump_count() const { return _jump_count; }

    size_t get_context_size() const override { return 0; }

    void transit_to(ITaskContext *target) override
    {
        // 同步测试中，transit_to 等同于让目标 jump_to
        target->jump_to();
    }

    void setup_flow(void (*entry)(), void *stack_top) override
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