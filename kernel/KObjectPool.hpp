#pragma once

#include "ObjectFactory.hpp"

template <typename T>
class KObjectPool
{
private:
    struct FreeNode
    {
        FreeNode *next;
    };

    FreeNode *_free_list = nullptr;
    ObjectFactory *_factory;
    size_t _object_size;

public:
    KObjectPool(ObjectFactory *f) : _factory(f)
    {
        // 确保空间足够存放指针，用于构建 FreeList
        size_t base_size = sizeof(T) > sizeof(FreeNode) ? sizeof(T) : sizeof(FreeNode);

        // 强制 16 字节对齐
        _object_size = (base_size + 15ULL) & ~15ULL;
    }

    T *allocate()
    {
        if (_free_list)
        {
            // 从空闲链表头部取出一个
            T *ptr = (T *)_free_list;
            _free_list = _free_list->next;
            // 注意：需要手动执行构造函数（如果有）
            return new (ptr) T();
        }

        // 空闲链表没了，向工厂申请新的
        return (T *)_factory->allocate_raw(_object_size);
    }

    void deallocate(T *ptr)
    {
        if (!ptr)
            return;

        // 执行析构函数（可选）
        ptr->~T();

        // 强转为 FreeNode 并插入空闲链表头部
        FreeNode *node = (FreeNode *)ptr;
        node->next = _free_list;
        _free_list = node;
    }
};