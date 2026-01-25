#pragma once

#include <cstdint>

// 与汇编约定的寄存器布局
struct TaskContext
{
    uint64_t r15, r14, r13, r12, rsi, rdi, rbx, rbp;
    uint64_t rip;
};

class Task
{
public:
    void *stack_pointer;
    uint32_t id;

    Task(uint32_t tid) : id(tid), stack_pointer(nullptr) {}

    // 纯内存操作：在栈中准备初始上下文
    void prepare_context(void *stack_top, void (*entry)())
    {
        auto *ctx = (TaskContext *)((uintptr_t)stack_top - sizeof(TaskContext));
        ctx->rip = (uintptr_t)entry;
        ctx->rbp = 0;
        this->stack_pointer = ctx;
    }
};