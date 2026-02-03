#pragma once

#include <common/Message.hpp>
#include "MessageCallback.hpp"
#include "IMessageBus.hpp"
#include "KList.hpp"
#include "KPoolList.hpp"

class MessageBus : public IMessageBus
{
private:
    IObjectBuilder *_builder;

    // 订阅者条目：管理特定消息类型的所有回调
    struct SubscriberEntry
    {
        MessageType type;
        KList<MessageCallback> callbacks;
        // 修正：构造函数接收 Builder 以初始化内部 KList
        SubscriberEntry(IObjectBuilder *b) : type(MessageType::NONE), callbacks(b) {}
    };

    KList<SubscriberEntry *> _registry;

    KObjectPool<ListNode<Message>> _queue_pool;
    KPoolList<Message> _pending_queue;

public:
    // 构造函数：统一使用 IObjectBuilder
    MessageBus(IObjectBuilder *b)
        : _builder(b), _registry(b), _queue_pool(b), _pending_queue(_queue_pool) // _registry 也是 KList，需要 builder
    {
    }

    ~MessageBus() override
    {
        // 1. 清空待处理队列
        _pending_queue.clear();

        // 2. 释放所有订阅者条目对象（因为它们是 construct 出来的）
        _registry.for_each([this](SubscriberEntry *entry)
                           { _builder->destroy(entry); });
        _registry.clear();
    }

    // --- IMessageBus 实现 ---

    void subscribe(MessageType type, MessageCallback callback) override
    {
        auto *entry = find_or_create_entry(type);
        if (entry)
        {
            entry->callbacks.push_back(callback);
        }
    }

    void unsubscribe(MessageType type, MessageCallback callback) override
    {
        auto *entry = find_entry(type);
        if (entry)
        {
            // 修正：KList 应提供 remove_if 逻辑
            entry->callbacks.remove_match([&callback](const MessageCallback &cb)
                                          { return cb == callback; });
        }
    }

    void publish(const Message &msg) override
    {

        _pending_queue.push_back(msg);
    }

    // --- 业务扩展逻辑 ---

    void subscribe_task(MessageType type, uint32_t task_id)
    {
        // 将 task_id 存入 context
        this->subscribe(type, MessageCallback(
                                  [](const Message &m, void *ctx)
                                  {
                                      uint32_t tid = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ctx));
                                      // 这里通常调用内核全局分发接口，例如：
                                      // Kernel::get_instance()->deliver_to_task(tid, m);
                                  },
                                  reinterpret_cast<void *>(static_cast<uintptr_t>(task_id))));
    }

    void dispatch_messages()
    {
        Message msg;
        // 修正：pop_front 是成员函数指针调用
        while (_pending_queue.pop_front(msg))
        {
            auto *entry = find_entry(msg.type);
            if (entry)
            {
                entry->callbacks.for_each([&msg](const MessageCallback &cb)
                                          { cb.invoke(msg); });
            }
        }
    }

private:
    SubscriberEntry *find_entry(MessageType type)
    {
        return _registry.find_match([type](SubscriberEntry *e)
                                    { return e->type == type; });
    }

    SubscriberEntry *find_or_create_entry(MessageType type)
    {
        auto *entry = find_entry(type);
        if (!entry)
        {
            // 修正：使用 _builder 替代已废弃的 _obj_factory
            entry = _builder->construct<SubscriberEntry>(_builder);
            if (entry)
            {
                entry->type = type;
                _registry.push_back(entry);
            }
        }
        return entry;
    }
};