#pragma once

#include <cstdint>

class IObjectFactory
{
public:
    virtual ~IObjectFactory() = default;

    /**
     * 基础内存分配（类似 malloc）
     */
    virtual void *allocate_raw(size_t size) = 0;

    /**
     * 基础内存释放（类似 free）
     */
    virtual void deallocate_raw(void *ptr, size_t size) = 0;

    /**
     * 模板化对象创建（构造函数注入）
     */
    template <typename T, typename... Args>
    T *create(Args &&...args)
    {
        void *ptr = allocate_raw(sizeof(T));
        if (!ptr)
            return nullptr;
        // 使用 placement new 在指定位置调用构造函数
        return new (ptr) T(static_cast<Args &&>(args)...);
    }

    /**
     * 模板化对象销毁（析构函数调用）
     */
    template <typename T>
    void destroy(T *ptr)
    {
        if (!ptr)
            return;
        ptr->~T(); // 调用析构函数
        deallocate_raw(ptr, sizeof(T));
    }
};