#pragma once

#include "ITaskLifecycle.hpp"
#include "IObjectBuilder.hpp"
#include "ITaskControlBlockFactory.hpp"
#include "KList.hpp"

class SimpleTaskLifecycle : public ITaskLifecycle
{
private:
    IObjectBuilder *_builder;               // 统一使用 Builder 替代 Factory
    ITaskControlBlockFactory *_tcb_factory; // 用于生产 TCB

    // 追踪所有任务（包括就绪、阻塞、挂起的）
    KList<ITaskControlBlock *> _all_tasks;
    size_t _task_count = 0;

    ITaskControlBlock *_current_task = nullptr;

public:
    // 修复构造函数：匹配 k_builder 和 tcb_factory (即你传入的 strategy/factory)
    SimpleTaskLifecycle(IObjectBuilder *builder, ITaskControlBlockFactory *tcb_factory)
        : _builder(builder),
          _tcb_factory(tcb_factory),
          _all_tasks(builder) // KList 需要 builder 来分配 Node
    {
    }

    // --- ITaskLifecycle 实现 ---
    ITaskControlBlock *spawn_task(const TaskExecutionInfo &exec,
                                  const TaskResourceConfig &res) override
    {
        // 1. 利用 TCB 工厂创建实体
        ITaskControlBlock *tcb = _tcb_factory->create_tcb(exec, res);
        if (!tcb)
            return nullptr;

        // 2. 纳入管理范围
        register_task(tcb);

        return tcb;
    }

    void destroy_task(ITaskControlBlock *tcb) override
    {
        if (!tcb)
            return;

        // 使用你现有的 remove_match 成员函数
        // predicate 逻辑：如果链表中的数据等于传入的 tcb 指针，则移除
        _all_tasks.remove_match([tcb](ITaskControlBlock *t)
                                { return t == tcb; });

        _task_count--;

        // 之后由统一回收模块处理 tcb 及其关联的 KObject 资源
    }

    void register_task(ITaskControlBlock *tcb) override
    {
        if (!tcb)
            return;
        _all_tasks.push_back(tcb);
        _task_count++;
    }

    ITaskControlBlock *get_task(uint32_t task_id) override
    {
        return _all_tasks.find_match([task_id](ITaskControlBlock *t)
                                     { return t->get_id() == task_id; });
    }

    ITaskControlBlock *get_current_task() const override { return _current_task; }
    size_t get_task_count() const override { return _task_count; }

    // 供调度引擎切换上下文时更新
    void set_current_task(ITaskControlBlock *tcb) { _current_task = tcb; }
};