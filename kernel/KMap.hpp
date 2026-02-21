#pragma once

#include <string.h>

template <typename K, typename V>
struct KPair
{
    K key;
    V value;
};

template <typename K, typename V>
struct KNode
{
    KPair<K, V> data;
    KNode *next;
    KNode *prev;
};

template <typename K, typename V, int MAX_SIZE = 64>
class KMap
{
private:
    struct KNode
    {
        K key;
        V value;
        bool occupied = false;
    };

    KNode _pool[MAX_SIZE];
    int _count = 0;

public:
    // 插入或更新
    bool insert(K key, V value)
    {
        // 1. 尝试查找现有 Key 并更新
        for (int i = 0; i < MAX_SIZE; i++)
        {
            if (_pool[i].occupied && is_equal(_pool[i].key, key))
            {
                _pool[i].value = value;
                return true;
            }
        }

        // 2. 找空位插入
        for (int i = 0; i < MAX_SIZE; i++)
        {
            if (!_pool[i].occupied)
            {
                _pool[i].key = key;
                _pool[i].value = value;
                _pool[i].occupied = true;
                _count++;
                return true;
            }
        }
        return false; // 池满
    }

    // 查询
    V *find(K key)
    {
        for (int i = 0; i < MAX_SIZE; i++)
        {
            if (_pool[i].occupied && is_equal(_pool[i].key, key))
            {
                return &_pool[i].value;
            }
        }
        return nullptr;
    }

private:
    // 针对内核常用的 const char* 进行特化比较
    bool is_equal(const char *a, const char *b)
    {
        return strcmp(a, b) == 0;
    }

    template <typename T>
    bool is_equal(const T &a, const T &b)
    {
        return a == b;
    }
};