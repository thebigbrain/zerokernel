#pragma once

#include <common/Message.hpp>
#include "MessageCallback.hpp"

class IMessageBus
{
public:
    virtual ~IMessageBus() = default;

    // 发布消息
    virtual void publish(const Message &msg) = 0;

    // 订阅消息（通常配合回调接口或 Lambda）
    virtual void subscribe(MessageType type, MessageCallback cb) = 0;

    // 取消订阅
    virtual void unsubscribe(MessageType type, MessageCallback cb) = 0;

    virtual void dispatch_messages() = 0;
};