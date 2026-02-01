#include "MessageBus.hpp"

MessageBus::MessageBus(ObjectFactory *f) : _factory(f)
{
    // 初始化消息节点池和待处理队列
    _msg_node_pool = f->create<KObjectPool<ListNode<Message>>>(f);
    _pending_queue = f->create<KPoolList<Message>>(_msg_node_pool);
    _registry = f->create<KList<SubscriberEntry *>>(f);
}

void MessageBus::subscribe(MessageType type, ITaskControlBlock *task)
{
    SubscriberEntry *entry = find_or_create_entry(type);
    entry->tasks->push_back(task);
}

void MessageBus::subscribe(MessageType type, KernelCallback callback)
{
    SubscriberEntry *entry = find_or_create_entry(type);
    entry->funcs->push_back(callback);
}

void MessageBus::publish(const Message &msg)
{
    _pending_queue->push_back(msg);
}

void MessageBus::dispatch_messages()
{
    if (_pending_queue->empty())
        return;

    auto it = _pending_queue->begin();
    auto end_it = _pending_queue->end();

    while (it != end_it)
    {
        const Message &msg = *it;
        SubscriberEntry *entry = find_entry(msg.type);

        if (entry)
        {
            // 1. 分发给订阅的任务控制块
            for (auto t_it = entry->tasks->begin(); t_it != entry->tasks->end(); ++t_it)
            {
                // 使用我们重构后的领域方法，这会自动处理 BLOCKED 状态唤醒
                (*t_it)->deliver(msg);
            }

            // 2. 执行内核级回调
            for (auto f_it = entry->funcs->begin(); f_it != entry->funcs->end(); ++f_it)
            {
                (*f_it).invoke(msg);
            }
        }
        ++it;
    }

    // 处理完毕，清空队列并交还节点给池
    _pending_queue->clear();
}

uint32_t MessageBus::get_pending_count()
{
    return _pending_queue->size();
}

SubscriberEntry *MessageBus::find_entry(MessageType type)
{
    for (auto node = _registry->begin(); node != _registry->end(); ++node)
    {
        if ((*node)->type == type)
            return *node;
    }
    return nullptr;
}

SubscriberEntry *MessageBus::find_or_create_entry(MessageType type)
{
    SubscriberEntry *e = find_entry(type);
    if (e)
        return e;

    // 创建新条目
    e = (SubscriberEntry *)_factory->allocate_raw(sizeof(SubscriberEntry));
    e->type = type;

    // 使用工厂创建内部列表，确保内存来源一致
    e->tasks = _factory->create<KList<ITaskControlBlock *>>(_factory);
    e->funcs = _factory->create<KList<KernelCallback>>(_factory);

    _registry->push_back(e);
    return e;
}