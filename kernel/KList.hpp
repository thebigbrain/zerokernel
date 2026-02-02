#pragma once

#include "ObjectFactory.hpp"

template <typename T>
struct ListNode
{
    T data;
    ListNode *next;
};

template <typename T>
struct Iterator
{
    ListNode<T> *current;

    // 允许 if(it) 或 while(it) 判断
    operator bool() const { return current != nullptr; }

    // 也要支持 -> 操作符方便访问 node 内部的 next
    // ListNode<T> *operator->() { return current; }

    T &operator*() { return current->data; }

    T &operator->() { return current->data; }

    Iterator &operator++()
    {
        if (current)
            current = current->next;
        return *this;
    }
};

template <typename T>
class KList
{
private:
    ListNode<T> *_head = nullptr;
    ObjectFactory *_obj_factory;

public:
    KList(ObjectFactory *f) : _obj_factory(f) {}

    void push_back(const T &data)
    {
        auto *node = (ListNode<T> *)_obj_factory->allocate_raw(sizeof(ListNode<T>));
        node->data = data;
        node->next = nullptr;

        if (!_head)
        {
            _head = node;
            return;
        }
        ListNode<T> *curr = _head;
        while (curr->next)
            curr = curr->next;
        curr->next = node;
    }

    Iterator<T> begin() { return Iterator<T>{_head}; }
    Iterator<T> end() { return Iterator<T>{nullptr}; }

    void clear()
    {
        // 在内核开发中，如果 factory 不支持 free，这里只能重置指针
        // 如果支持释放，则需要循环调用 factory->deallocate
        _head = nullptr;
    }

    /**
     * 谓词查找：接受一个返回 bool 的函数
     * 例如：list.find([](const T& item) { return item->id == 123; });
     */
    template <typename F>
    T *find(F predicate)
    {
        ListNode<T> *curr = _head;
        while (curr)
        {
            if (predicate(curr->data))
            {
                return &(curr->data); // 返回数据项的指针，方便修改或获取
            }
            curr = curr->next;
        }
        return nullptr; // 没找到
    }

    /**
     * 针对指针类型的查找优化版（直接返回指针本身，不返回二级指针）
     */
    template <typename F>
    T find_match(F predicate)
    {
        ListNode<T> *curr = _head;
        while (curr)
        {
            if (predicate(curr->data))
            {
                return curr->data;
            }
            curr = curr->next;
        }
        return T(); // 如果是指针则返回 nullptr
    }
};