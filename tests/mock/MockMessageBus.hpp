#pragma once

#include "kernel/IMessageBus.hpp"
#include "common/Message.hpp"

class MockMessageBus : public IMessageBus
{
public:
    MessageType last_published_type = MessageType::NONE;
    bool publish_called = false;

    void publish(const Message &msg) override
    {
        publish_called = true;
        last_published_type = msg.type;
        // 如果需要，可以在这里校验 msg.data 里的字符串内容
    }

    // 单元测试中暂时不需要实现 subscribe/unsubscribe
    void subscribe(MessageType type, MessageCallback cb) override {}
    void unsubscribe(MessageType type, MessageCallback cb) override {}
};