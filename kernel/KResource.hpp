#pragma once

#include "IAllocator.hpp"

/**
 * 通用资源包装器
 * 用于将原始内存需求转化为受管对象
 */
template <typename T>
class KResource
{
private:
    T *_ptr;
    size_t _count;
    IAllocator *_allocator;

public:
    // 构造时直接分配内存
    KResource(IAllocator *alloc, size_t count, size_t alignment = 16)
        : _count(count), _allocator(alloc)
    {
        _ptr = static_cast<T *>(_allocator->allocate(count * sizeof(T), alignment));
    }

    // 析构时自动释放，确保“万物皆对象”后的资源自愈
    ~KResource()
    {
        if (_ptr && _allocator)
        {
            _allocator->deallocate(_ptr, _count * sizeof(T));
        }
    }

    // 禁止拷贝，防止双重释放（内核安全准则）
    KResource(const KResource &) = delete;
    KResource &operator=(const KResource &) = delete;

    // 资源访问接口
    T *get() const { return _ptr; }
    size_t count() const { return _count; }
    size_t size_in_bytes() const { return _count * sizeof(T); }

    // 方便像数组一样使用
    T &operator[](size_t index) { return _ptr[index]; }
};