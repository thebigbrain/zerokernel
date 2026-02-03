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
    using IObjectBuilder::IObjectBuilder;

    template <typename T, typename... Args>
    T *construct(Args &&...args)
    {
        T *ptr = IObjectBuilder::construct<T>(args...);
        if (ptr)
        {
            _active_objects++; // 每次成功创建，计数加一
            // 甚至可以记录 T 的名称，用于内核调试输出 (ls-objects)
        }
        return ptr;
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