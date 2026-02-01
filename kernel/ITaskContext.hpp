#pragma once

#include <cstdint>

/**
 * 体系结构无关的上下文句柄
 * 内核只看到一个指针，具体内容由底层的汇编原语解释
 */
class ITaskContext
{
public:
    virtual ~ITaskContext() = default;

    // 内核只调用这些具有语义的方法
    virtual void prepare(void (*entry)(), void *stack_top, void (*exit_router)()) = 0;

    virtual void set_parameter(int index, uintptr_t value) = 0;

    // 某些情况下内核可能需要知道栈指针，但不需要知道内部布局
    virtual void *get_stack_pointer() const = 0;
    virtual void set_stack_pointer(void *sp) = 0;
};