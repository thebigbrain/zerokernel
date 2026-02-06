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

    virtual size_t get_context_size() const = 0;

    /**
     * 核心动作：从当前执行流切换到另一个执行流
     * @param target 目标上下文
     * 内部实现：保存当前寄存器到 this，从 target 恢复寄存器并跳转
     */
    virtual void transit_to(ITaskContext *target) = 0;

    /**
     * 配置执行流的基础要素
     * @param entry 任务入口点
     * @param stack_top 栈顶指针（通常是分配好的内存最高处）
     * @param exit_stub 任务结束后的回归地址（内核路由）
     */
    virtual void setup_flow(void (*entry)(void *, void *), void *stack_top) = 0;

    /**
     * 载入初始化参数
     * @param index 参数位置（0 通常对应第一个参数）
     * @param value 参数的值（指针或整数）
     */
    virtual void load_argument(size_t index, uintptr_t value) = 0;

    /**
     * 获取当前的栈指针
     * 调度器在保存上下文时可能需要此值来更新 TCB 中的记录
     */
    virtual void *get_stack_pointer() const = 0;
};