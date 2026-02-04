#pragma once
#include "kernel.hpp"

/**
 * @brief KernelInspector: 专为测试和底层诊断设计的内核内部状态访问器
 */
class KernelInspector
{
private:
    const Kernel *_kernel;

public:
    explicit KernelInspector(const Kernel *k) : _kernel(k) {}

    // 检查内核是否有效
    bool is_valid() const { return _kernel != nullptr; }

    // 基础设施访问
    IMessageBus *get_message_bus() const { return _kernel->_bus; }
    ITaskLifecycle *get_task_lifecycle() const { return _kernel->_lifecycle; }
    TaskService *get_task_service() const { return _kernel->_task_service; }
    ICPUEngine *get_cpu() const { return _kernel->_cpu; }
    IAllocator *get_runtime_heap() const { return _kernel->_runtime_heap; }
    IObjectBuilder *get_object_builder() const { return _kernel->_builder; }
    ISchedulingStrategy *get_scheduling_strategy() const { return _kernel->_strategy; }
    IExecutionEngine *get_execution_engine() const { return _kernel->_engine; }

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