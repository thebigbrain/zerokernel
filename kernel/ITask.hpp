#pragma once

#include <common/Message.hpp>

class ITask
{
public:
    virtual void on_message(const Message &msg) = 0;
    virtual uint32_t get_id() const = 0;
    virtual void push_message(const Message &msg) = 0;
    virtual bool has_message() = 0;
    virtual Message pop_message() = 0;
};