#include "SimpleTaskFactory.hpp"
#include "SimpleTaskControlBlock.hpp"
#include "KernelUtils.hpp"
#include "KStackBuffer.hpp"

ITaskControlBlock *SimpleTaskFactory::create_tcb(
    const TaskExecutionInfo &exec_info,
    const TaskResourceConfig &res_config)
{
    // 1. 分配任务 ID
    auto id = _id_gen->acquire();

    // 2. 利用注入的 ContextFactory 创建上下文对象
    // 注意：上下文对象通常是协议栈或 CPU 寄存器状态的抽象
    ITaskContext *ctx = _context_factory->create_context();
    if (!ctx)
    {
        _id_gen->release(id);
        return nullptr;
    }

    auto stack = res_config.stack;
    // 使用 _builder 获取底层分配器来申请原始字节流
    if (!stack)
    { /* 错误处理... */
        _context_factory->destroy_context(ctx);
        _id_gen->release(id);
        return nullptr;
    }

    // 5. 初始化上下文 (Setup Execution Flow)
    // 注入入口点、对齐后的栈顶、以及任务退出时的跳转地址
    ctx->setup_flow(
        reinterpret_cast<void (*)()>(exec_info.entry),
        stack->get_aligned_top());

    // 注入参数：ABI 约定
    // 第一个参数通常存放 Runtime 指针，第二个参数存放任务配置
    ctx->load_argument(0, reinterpret_cast<uintptr_t>(exec_info.runtime));
    ctx->load_argument(1, reinterpret_cast<uintptr_t>(exec_info.config));

    // 6. 最终封装为 TCB 对象
    // 使用 _builder 构造 TCB，这样 TCB 内部如果需要动态分配内存也能追踪
    return _builder->construct<SimpleTaskControlBlock>(id, ctx, exec_info, res_config);
}