#pragma once

#include "IObjectBuilder.hpp"

template <typename T>
struct ListNode
{
    T data;
    ListNode *next;

    // 增加构造函数，方便 Builder 调用
    ListNode(const T &d) : data(d), next(nullptr) {}
};

template <typename T>
struct Iterator
{
    ListNode<T> *current;

    T &operator*() { return current->data; }
    T *operator->() { return &(current->data); }

    Iterator &operator++()
    {
        if (current)
            current = current->next;
        return *this;
    }

    bool operator!=(const Iterator &other) const { return current != other.current; }
};

template <typename T>
class KList
{
private:
    ListNode<T> *_head = nullptr;
    ListNode<T> *_tail = nullptr;
    IObjectBuilder *_builder; // 迁移至抽象构建器

public:
    KList(IObjectBuilder *b) : _builder(b) {}

    // 析构时必须清理，防止内核内存泄漏
    ~KList()
    {
        clear();
    }

    void push_back(const T &data)
    {
        // 使用 construct 确保 T 的构造函数被调用（即使 T 是复杂对象）
        auto *node = _builder->construct<ListNode<T>>(data);
        if (!node)
            return;

        if (!_tail)
        {
            _head = _tail = node;
        }
        else
        {
            _tail->next = node;
            _tail = node;
        }
    }

    template <typename F>
    void for_each(F action)
    {
        ListNode<T> *curr = _head;
        while (curr)
        {
            action(curr->data);
            curr = curr->next;
        }
    }

    template <typename Compare>
    void push_sorted(const T &data, Compare comp)
    {
        auto *new_node = _builder->construct<ListNode<T>>(data);
        if (!new_node)
            return;

        if (!_head || comp(data, _head->data))
        {
            new_node->next = _head;
            if (!_head)
                _tail = new_node; // 链表原本为空
            _head = new_node;
            return;
        }

        ListNode<T> *curr = _head;
        while (curr->next && !comp(data, curr->next->data))
        {
            curr = curr->next;
        }

        new_node->next = curr->next;
        if (!curr->next)
            _tail = new_node; // 插入到了末尾
        curr->next = new_node;
    }

    void pop_front()
    {
        if (!_head)
            return;

        ListNode<T> *old_node = _head;
        _head = _head->next;

        if (!_head)
            _tail = nullptr;

        // 使用 destroy 确保 T 的析构函数被调用并回收内存
        _builder->destroy(old_node);
    }

    // 增加：移除匹配项，MessageBus unsubscribe 需要它
    template <typename F>
    void remove_match(F predicate)
    {
        ListNode<T> *curr = _head;
        ListNode<T> *prev = nullptr;

        while (curr)
        {
            if (predicate(curr->data))
            {
                ListNode<T> *to_delete = curr;
                if (prev)
                {
                    prev->next = curr->next;
                }
                else
                {
                    _head = curr->next;
                }

                if (curr == _tail)
                    _tail = prev;

                curr = curr->next;
                _builder->destroy(to_delete);
            }
            else
            {
                prev = curr;
                curr = curr->next;
            }
        }
    }

    void clear()
    {
        while (!empty())
        {
            pop_front();
        }
    }

    T front() const { return _head ? _head->data : T(); }
    Iterator<T> begin() { return Iterator<T>{_head}; }
    Iterator<T> end() { return Iterator<T>{nullptr}; }
    bool empty() const { return _head == nullptr; }

    template <typename F>
    T find_match(F predicate) const
    {
        ListNode<T> *curr = _head;
        while (curr)
        {
            if (predicate(curr->data))
                return curr->data;
            curr = curr->next;
        }
        return T();
    }
};