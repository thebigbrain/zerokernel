#pragma once

#include "KObjectPool.hpp"
#include "KList.hpp" // 复用 ListNode 定义

template <typename T>
class KPoolList
{
private:
    ListNode<T> *_head = nullptr;
    ListNode<T> *_tail = nullptr;
    KObjectPool<ListNode<T>> *_pool;

    uint32_t _size = 0;

public:
    // 强制要求传入对象池
    KPoolList(KObjectPool<ListNode<T>> *pool) : _pool(pool) {}

    void push_back(const T &data)
    {
        // 从池中获取节点
        ListNode<T> *node = _pool->allocate();
        node->data = data;
        node->next = nullptr;

        if (!_head)
        {
            _head = _tail = node;
        }
        else
        {
            _tail->next = node;
            _tail = node;
        }

        _size++;
    }

    // 清空列表并将所有节点还给对象池
    void clear()
    {
        ListNode<T> *curr = _head;
        while (curr)
        {
            ListNode<T> *next = curr->next;
            _pool->deallocate(curr); // 归还节点
            curr = next;
        }
        _head = _tail = nullptr;
        _size = 0;
    }

    // 提供给 MessageBus 遍历使用的裸指针接口
    Iterator<T> begin() { return Iterator<T>{_head}; }
    Iterator<T> end() { return Iterator<T>{nullptr}; }

    // 简单判断是否为空
    bool empty() const { return _head == nullptr; }

    uint32_t size() const
    {
        return _size;
    }
};