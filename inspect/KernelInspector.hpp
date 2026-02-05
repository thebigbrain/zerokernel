#pragma once

#include <kernel/Kernel.hpp>

/**
 * @brief KernelInspector: 专为测试和底层诊断设计的内核内部状态访问器
 */
class KernelInspector
{
private:
    Kernel *_kernel;
    // 记录已经执行过的任务载荷（TCB 指针）
    KList<uint32_t> _executed_ids;

public:
    explicit KernelInspector(Kernel *k) : _kernel(k), _executed_ids(k->_builder)
    {
    }

    /**
     * @brief 核心查询方法：基于 ID 检查任务是否运行过
     */
    bool is_task_executed(uint32_t task_id)
    {
        for (auto it = _executed_ids.begin(); it != _executed_ids.end(); ++it)
        {
            if (*it == task_id)
            {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief 在 KernelInspector 中利用友元权限实现
     */
    bool is_task_executed_by_name(const char *name)
    {
        if (!name || !_kernel || !_kernel->_task_archives)
        {
            return false;
        }

        uint32_t target_id = 0;
        bool found_archive = false;

        // 1. 直接访问 Kernel 的私有成员 _task_archives
        // 寻找匹配名称的任务 ID
        for (auto it = _kernel->_task_archives->begin(); it != _kernel->_task_archives->end(); ++it)
        {
            if (strcmp((*it).name, name) == 0)
            {
                target_id = (*it).id;
                found_archive = true;
                break;
            }
        }

        if (!found_archive)
            return false;

        // 2. 在 Inspector 自身的执行历史中查找该 ID
        return is_task_executed(target_id);
    }

    void reset()
    {
        _executed_ids.clear();
    }

    // 检查内核是否有效
    bool is_valid() const { return _kernel != nullptr; }

    // 基础设施访问
    IMessageBus *bus() const { return _kernel->_bus; }
    ITaskLifecycle *lifecycle() const { return _kernel->_lifecycle; }
    TaskService *task_service() const { return _kernel->_task_service; }
    IAllocator *heap() const { return _kernel->_runtime_heap; }
    IObjectBuilder *builder() const { return _kernel->_builder; }
    ISchedulingStrategy *strategy() const { return _kernel->_strategy; }
    ISchedulingControl *control() const { return _kernel->_platform_hooks->sched_control; }

    /**
     * @brief 在 Mock 类中补全任务创建逻辑
     */
    ITaskControlBlock *create_task(TaskEntry entry, TaskPriority priority, const char *name)
    {
        return _kernel->create_kernel_task(entry, priority, 4096, nullptr, name);
    }

    const TaskArchive *find_archive_by_entry(TaskEntry entry) const
    {
        auto *_task_archives = _kernel->_task_archives;
        if (!_task_archives)
            return nullptr;

        for (auto it = _task_archives->begin(); it != _task_archives->end(); ++it)
        {
            if ((*it).entry == entry)
                return &(*it);
        }
        return nullptr;
    }

    // 辅助方法：快速检查堆水位线
    size_t get_heap_free_size() const
    {
        if (!_kernel->_runtime_heap)
            return 0;
        // 假设你的堆分配器也有类似 StaticLayoutAllocator 的探测方法
        // 或者通过转型来获取具体数据
        return 0; // 视具体实现而定
    }
};