#pragma once

#include "KObjectPool.hpp"
#include "KList.hpp"

template <typename T>
class KPoolList
{
private:
    ListNode<T> *_head = nullptr;
    ListNode<T> *_tail = nullptr;
    KObjectPool<ListNode<T>> &_pool;

    uint32_t _size = 0;

public:
    /**
     * 构造函数：注入 Builder，自建专属对象池
     */
    KPoolList(KObjectPool<ListNode<T>> &pool) : _pool(pool)
    {
        // 不再需要 Builder，因为池子已经由外部用 Builder 建好了
    }

    ~KPoolList()
    {
        clear();
    }

    void push_back(const T &data)
    {
        // 1. 从池中获取一个裸内存节点
        ListNode<T> *node = _pool.acquire(data);
        if (!node)
            return;

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

    /**
     * pop_front: 获取并移除头部元素
     */
    bool pop_front(T &out_data)
    {
        if (!_head)
            return false;

        ListNode<T> *node_to_remove = _head;

        // 拷贝数据
        out_data = node_to_remove->data;

        _head = _head->next;
        if (!_head)
        {
            _tail = nullptr;
        }

        // 归还节点给池（池内部只是标记可用，不触发 T 的析构，除非你在池里实现了它）
        _pool.release(node_to_remove);
        _size--;

        return true;
    }

    void clear()
    {
        ListNode<T> *curr = _head;
        while (curr)
        {
            ListNode<T> *next = curr->next;
            _pool.release(curr);
            curr = next;
        }
        _head = _tail = nullptr;
        _size = 0;
    }

    Iterator<T> begin() { return Iterator<T>{_head}; }
    Iterator<T> end() { return Iterator<T>{nullptr}; }

    bool empty() const { return _head == nullptr; }
    uint32_t size() const { return _size; }
};