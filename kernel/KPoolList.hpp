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

    /**
     * pop_front: 获取并移除头部元素
     * @param out_data 输出参数，用于存放弹出的数据
     * @return bool 如果列表为空返回 false，成功弹出返回 true
     */
    bool pop_front(T &out_data)
    {
        if (!_head)
        {
            return false;
        }

        // 1. 暂存当前头部节点
        ListNode<T> *node_to_remove = _head;

        // 2. 拷贝数据到输出变量
        out_data = node_to_remove->data;

        // 3. 移动头指针
        _head = _head->next;

        // 4. 如果头变空了，尾也要置空
        if (!_head)
        {
            _tail = nullptr;
        }

        // 5. 【核心】将节点归还给对象池，防止内存泄漏
        _pool->deallocate(node_to_remove);

        // 6. 更新计数
        _size--;

        return true;
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