#include "MessageBus.hpp"

MessageBus::MessageBus(ObjectFactory *f) : _obj_factory(f)
{
    // 初始化消息节点池和待处理队列
    _msg_node_pool = f->create<KObjectPool<ListNode<Message>>>(f);
    _pending_queue = f->create<KPoolList<Message>>(_msg_node_pool);
    _registry = f->create<KList<SubscriberEntry *>>(f);
}

void MessageBus::subscribe(MessageType type, uint32_t task_id)
{
    SubscriberEntry *entry = find_or_create_entry(type);
    entry->task_ids->push_back(task_id);
}

// 订阅：内核回调（用于内核模块内部通信）
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
    Message msg;
    // 1. 从就绪队列不断取出待处理的消息
    while (_pending_queue->pop_front(msg))
    {
        SubscriberEntry *entry = find_entry(msg.type);
        if (!entry)
            continue;

        // 2. 分发给所有订阅的 Task ID
        for (auto it = entry->task_ids->begin(); it != entry->task_ids->end(); ++it)
        {
            // 通过具体的投递逻辑将消息送往任务邮箱
            deliver_to_task(*it, msg);
        }

        // 3. 执行所有订阅的内核回调
        for (auto it = entry->funcs->begin(); it != entry->funcs->end(); ++it)
        {
            (*it).invoke(msg);
        }
    }
}
void MessageBus::deliver_to_task(uint32_t target_id, const Message &msg)
{
    // 1. 找到对应的消息条目
    SubscriberEntry *entry = find_entry(msg.type);
    if (!entry)
        return;

    // 2. 验证该任务是否在订阅列表中
    // 再次利用 KList 的查找能力
    uint32_t *found_id = entry->task_ids->find([target_id](uint32_t id)
                                               { return id == target_id; });

    if (found_id)
    {
        // 执行分发逻辑（例如放入任务的私有队列或触发中断模拟）
        // 这里体现了为什么要解耦：Bus 只管找到 ID，
        // 具体的“推送到任务”动作可以交给 Runtime 或 TaskManager
    }
}

uint32_t MessageBus::get_pending_count()
{
    return _pending_queue->size();
}

SubscriberEntry *MessageBus::find_entry(MessageType type)
{
    // 利用我们之前补充的 lambda 查找逻辑
    return _registry->find_match([type](SubscriberEntry *e)
                                 { return e->type == type; });
}

SubscriberEntry *MessageBus::find_or_create_entry(MessageType type)
{
    SubscriberEntry *entry = find_entry(type);
    if (entry)
        return entry;

    // 创建新条目：注意嵌套链表的初始化
    entry = (SubscriberEntry *)_obj_factory->allocate_raw(sizeof(SubscriberEntry));
    entry->type = type;
    entry->task_ids = _obj_factory->create<KList<uint32_t>>(_obj_factory);
    entry->funcs = _obj_factory->create<KList<KernelCallback>>(_obj_factory);

    _registry->push_back(entry);
    return entry;
}