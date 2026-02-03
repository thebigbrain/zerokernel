#pragma once

#include <utility>
#include <cstddef>
#include <new>

#include "IAllocator.hpp"
/**
 * 修复 C2661: 针对过度对齐对象的 Placement New 适配。
 * 编译器在处理 alignas(N) 的类进行 placement new 时，
 * 会尝试匹配带有 std::align_val_t 的版本。
 */
inline void *operator new(std::size_t size, std::align_val_t al, void *ptr) noexcept
{
    (void)size; // 抑制未使用变量警告
    (void)al;
    return ptr; // Placement new 仅仅是返回传入的地址
}

// 配套的 delete 也应补全，防止编译器在异常处理路径找不到符号
inline void operator delete(void *ptr, std::align_val_t al, void *place) noexcept
{
    (void)ptr;
    (void)al;
    (void)place;
}

class IObjectBuilder
{
protected:
    IAllocator *_allocator;

public:
    IObjectBuilder(IAllocator *alloc) : _allocator(alloc) {}
    virtual ~IObjectBuilder() = default;

    /**
     * 核心能力：在 Allocator 提供的空间上构建对象
     */
    template <typename T, typename... Args>
    T *construct(Args &&...args)
    {
        // 1. 调用 Allocator 分配空间
        void *ptr = _allocator->allocate(sizeof(T), alignof(T));
        if (!ptr)
            return nullptr;

        // 2. 执行 Placement New (Builder 的核心职责)
        return new (ptr) T(std::forward<Args>(args)...);
    }

    /**
     * 销毁对象并归还内存
     */
    template <typename T>
    void destroy(T *ptr)
    {
        if (!ptr)
            return;

        // 1. 显式调用析构函数
        ptr->~T();

        // 2. 归还内存给 Allocator
        _allocator->deallocate(ptr, sizeof(T));
    }
};