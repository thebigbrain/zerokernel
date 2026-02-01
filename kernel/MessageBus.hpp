#pragma once

#include <common/Message.hpp>
#include "ITask.hpp"
#include "KList.hpp"
#include "KPoolList.hpp"
#include "ObjectFactory.hpp"
#include "KObjectPool.hpp"
#include "KernelCallback.hpp"

struct SubscriberEntry
{
    MessageType type;
    KList<ITask *> *tasks;
    KList<KernelCallback> *funcs;
};

class MessageBus
{
private:
    ObjectFactory *_factory;

    // 注册表：存储不同消息类型的订阅关系
    KList<SubscriberEntry *> *_registry;

    // 对象池：专门管理消息节点的内存，防止 dispatch 频率过高导致内存耗尽
    KObjectPool<ListNode<Message>> *_msg_node_pool;

    // 待处理队列：暂存 publish 进来的消息
    KPoolList<Message> *_pending_queue;

public:
    MessageBus(ObjectFactory *f) : _factory(f)
    {
        // 1. 初始化消息节点对象池
        _msg_node_pool = f->create<KObjectPool<ListNode<Message>>>(f);

        // 2. 初始化待处理消息队列（传入对象池）
        _pending_queue = f->create<KPoolList<Message>>(_msg_node_pool);

        // 3. 初始化注册表（注册表节点较少，可以使用默认工厂分配或单独建池）
        _registry = f->create<KList<SubscriberEntry *>>(f);
    }

    // 核心：由内核调度器（如每个 Tick 或 Idle 时）调用
    void dispatch_messages()
    {
        // 使用 begin() 和 end() 进行遍历
        auto it = _pending_queue->begin();
        auto end_it = _pending_queue->end();

        while (it != end_it)
        {
            // 注意：这里的 it 是 Iterator 对象，需要通过 *it 获取数据
            const Message &msg = *it;
            SubscriberEntry *entry = find_entry(msg.type);

            if (entry)
            {
                // 分发给任务
                for (auto t_it = entry->tasks->begin(); t_it != entry->tasks->end(); ++t_it)
                {
                    (*t_it)->push_message(msg);
                }

                // 执行内核回调
                for (auto f_node = entry->funcs->begin(); f_node != entry->funcs->end(); ++f_node)
                {
                    (*f_node).invoke(msg);
                }
            }
            ++it; // 调用 Iterator 的 operator++
        }

        _pending_queue->clear();
    }

    // 订阅接口：任务订阅
    void subscribe(MessageType type, ITask *task)
    {
        SubscriberEntry *entry = find_or_create_entry(type);
        entry->tasks->push_back(task);
    }

    // 订阅接口：内核函数订阅
    void subscribe(MessageType type, KernelCallback callback)
    {
        SubscriberEntry *entry = find_or_create_entry(type);

        entry->funcs->push_back(callback);
    }

    // 发布接口：仅将消息压入待处理队列
    void publish(const Message &msg)
    {
        _pending_queue->push_back(msg);
    }

    uint32_t get_pending_count()
    {
        return _pending_queue->size();
    }

private:
    SubscriberEntry *find_or_create_entry(MessageType type)
    {
        SubscriberEntry *e = find_entry(type);
        if (e)
            return e;

        // 创建新条目
        e = (SubscriberEntry *)_factory->allocate_raw(sizeof(SubscriberEntry));
        e->type = type;

        // 为每个条目初始化列表
        // 注意：任务列表和函数列表通常变动频率低，直接用 factory 分配
        e->tasks = (KList<ITask *> *)_factory->allocate_raw(sizeof(KList<ITask *>));
        e->funcs = (KList<KernelCallback> *)_factory->allocate_raw(sizeof(KList<KernelCallback>));

        new (e->tasks) KList<ITask *>(_factory);
        new (e->funcs) KList<KernelCallback>(_factory);

        _registry->push_back(e);
        return e;
    }

    SubscriberEntry *find_entry(MessageType type)
    {
        auto node = _registry->begin();
        auto end_node = _registry->end();
        while (node != end_node)
        {
            if (node->type == type)
                return *node;

            ++node;
        }
        return nullptr;
    }
};