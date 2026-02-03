#pragma once

#include <utility>
#include "IObjectBuilder.hpp"

template <typename T>
class KObjectPool
{
private:
    struct FreeNode
    {
        FreeNode *next;
    };
    FreeNode *_free_list = nullptr;
    IObjectBuilder *_builder;
    size_t _object_size;

public:
    KObjectPool(IObjectBuilder *b) : _builder(b)
    {
        _object_size = sizeof(T) > sizeof(FreeNode) ? sizeof(T) : sizeof(FreeNode);
        _object_size = (_object_size + 15ULL) & ~15ULL; // 保持 16 字节对齐
    }

    // 返回已经构造好的 T*
    template <typename... Args>
    T *acquire(Args &&...args)
    {
        T *ptr = nullptr;
        if (_free_list)
        {
            ptr = reinterpret_cast<T *>(_free_list);
            _free_list = _free_list->next;
            // 在旧内存上重新触发构造函数
            return new (ptr) T(std::forward<Args>(args)...);
        }

        // 池子里没了，找 builder 真的生一个出来
        // 这样 builder 还能继续追踪这个对象的来源
        return _builder->construct<T>(std::forward<Args>(args)...);
    }

    void release(T *ptr)
    {
        if (!ptr)
            return;

        // 1. 显式调用析构，清理 T 的内部状态（比如 T 里的 KList）
        ptr->~T();

        // 2. 回收到空闲链表，不交还给底层分配器
        FreeNode *node = reinterpret_cast<FreeNode *>(ptr);
        node->next = _free_list;
        _free_list = node;
    }
};