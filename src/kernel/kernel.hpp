#pragma once
#include "task.hpp"

class Kernel
{
private:
    void *_raw_mem;
    size_t _mem_ptr = 0;
    void (*_hardware_switch)(void **, void *);

    Task *_current = nullptr;
    Task *_next = nullptr;

public:
    // 构造函数：内核承认线性内存的存在
    Kernel(BootInfo *info)
        : _raw_mem(info->memory_base),
          _hardware_switch(info->switcher) {}

    // 内核自己的内存分配逻辑（不依赖任何外部库）
    template <typename T, typename... Args>
    T *create_in_kernel(Args &&...args)
    {
        void *addr = (uint8_t *)_raw_mem + _mem_ptr;
        _mem_ptr += sizeof(T);
        return new (addr) T(std::forward<Args>(args)...);
    }

    void bootstrap()
    {
        // 创建任务对象
        _current = create_in_kernel<Task>(1);
        _next = create_in_kernel<Task>(2);

        // 划分栈空间
        void *stack_top = (uint8_t *)_raw_mem + (_mem_ptr + 16384);
        _mem_ptr += 16384;

        // 准备 Task 的 Context (完全在内存中操作结构体)
        _next->prepare_context(stack_top, []()
                               {
            while(true) { /* 任务逻辑 */ } });
    }

    void dispatch()
    {
        Task *prev = _current;
        _current = _next;
        _next = prev;
        // 调用由 Bootloader 准备好的底层原语
        _hardware_switch(&prev->stack_pointer, _current->stack_pointer);
    }

    void run()
    {
        bootstrap();
        dispatch();
    }
};