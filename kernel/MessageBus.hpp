#pragma once

#include <iostream>
#include <queue>
#include <map>
#include <windows.h>
#include <conio.h>
#include <functional>
#include <common/Message.hpp>

#include "ITask.hpp"

class MessageBus
{
private:
    // 订阅表：Key 是消息类型，Value 是订阅了该类型的 Task 指针列表
    std::map<MessageType, std::vector<ITask *>> _subscribers;
    std::map<MessageType, std::vector<std::function<void(const Message &)>>> _func_subscribers;

    // 全局待处理队列
    std::queue<Message> _global_queue;

public:
    // 订阅：任务告知总线它对某种类型的消息感兴趣
    void subscribe(MessageType type, ITask *task)
    {
        _subscribers[type].push_back(task);
    }

    void subscribe(MessageType type, std::function<void(const Message &)> callback)
    {
        _func_subscribers[type].push_back(callback);
    }

    // 发布：发送方丢出消息，不关心谁看
    void publish(Message msg)
    {
        _global_queue.push(msg);
    }

    // 分发逻辑：由内核在调度周期内调用
    void process_and_route()
    {
        while (!_global_queue.empty())
        {
            Message msg = _global_queue.front();
            _global_queue.pop();

            // 将消息推送到所有感兴趣的收件箱
            for (auto *task : _subscribers[msg.type])
            {
                task->push_message(msg);
            }

            // 执行所有函数回调
            for (auto &callback : _func_subscribers[msg.type])
            {
                callback(msg);
            }
        }
    }
};