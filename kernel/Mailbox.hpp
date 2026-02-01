#pragma once

#include <common/Message.hpp>
#include <vector>
#include <queue>
#include <mutex>

/**
 * Mailbox: 任务的“信箱”
 * 负责缓存发往该任务的消息
 */
class Mailbox
{
private:
    std::queue<Message> _messages;
    std::mutex _lock; // 在模拟器环境下使用 mutex 保证并发安全
    size_t _capacity;

public:
    explicit Mailbox(size_t capacity = 32) : _capacity(capacity) {}

    /**
     * 投递消息
     * @return 如果信箱已满则返回 false
     */
    bool push(const Message &msg)
    {
        std::lock_guard<std::mutex> lock(_lock);
        if (_messages.size() >= _capacity)
        {
            return false;
        }
        _messages.push(msg);
        return true;
    }

    /**
     * 取出消息
     */
    bool pop(Message &out_msg)
    {
        std::lock_guard<std::mutex> lock(_lock);
        if (_messages.empty())
        {
            return false;
        }
        out_msg = _messages.front();
        _messages.pop();
        return true;
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(_lock);
        return _messages.empty();
    }

    size_t count()
    {
        std::lock_guard<std::mutex> lock(_lock);
        return _messages.size();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(_lock);
        std::queue<Message> empty_queue;
        _messages.swap(empty_queue);
    }
};