#pragma once

#include <common/Message.hpp>
#include "ITaskControlBlock.hpp"
#include "KList.hpp"
#include "KPoolList.hpp"
#include "ObjectFactory.hpp"
#include "KObjectPool.hpp"
#include "KernelCallback.hpp"

/**
 * SubscriberEntry: 存储特定消息类型的订阅者（任务和内核回调）
 */
struct SubscriberEntry
{
    MessageType type;
    KList<uint32_t> *task_ids;
    KList<KernelCallback> *funcs;
};

/**
 * MessageBus: 内核消息中转站
 * 采用异步发布、同步分发的策略。通过对象池管理消息节点以提升性能。
 */
class MessageBus
{
private:
    ObjectFactory *_obj_factory;
    KList<SubscriberEntry *> *_registry;
    KObjectPool<ListNode<Message>> *_msg_node_pool;
    KPoolList<Message> *_pending_queue;

public:
    MessageBus(ObjectFactory *f);

    // 订阅接口
    void subscribe(MessageType type, uint32_t task_id);
    void subscribe(MessageType type, KernelCallback callback);

    // 发布接口
    void publish(const Message &msg);

    // 分发逻辑：由 Kernel::run_loop 调用
    void dispatch_messages();

    void deliver_to_task(uint32_t target_id, const Message &msg);

    uint32_t get_pending_count();

private:
    SubscriberEntry *find_entry(MessageType type);
    SubscriberEntry *find_or_create_entry(MessageType type);
};