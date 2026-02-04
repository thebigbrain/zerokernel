#pragma once

#include "IObjectBuilder.hpp"

/**
 * KernelObjectBuilder: 负责在内核空间构建对象
 */
class KernelObjectBuilder : public IObjectBuilder
{
private:
    size_t _active_objects = 0; // 内核存活对象计数

public:
    KernelObjectBuilder(IAllocator *alloc)
        : IObjectBuilder(alloc)
    {
    }

    // 增加一个虚函数确保虚表存在
    virtual ~KernelObjectBuilder() {}

    void on_object_created() override
    {
        _active_objects++;
        // 这里以后可以放 [2026-02-03] 要求的 OID 递增逻辑
        // std::cout << "Object created, total active: " << _active_objects << std::endl;
    }

    template <typename T>
    void destroy(T *ptr)
    {
        if (ptr)
        {
            IObjectBuilder::destroy(ptr);
            _active_objects--; // 销毁时减一
        }
    }
};