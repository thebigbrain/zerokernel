#pragma once

#include "Message.hpp"

class IUserRuntime
{
public:
    virtual void publish(const Message &msg) = 0;
    virtual void yield() = 0;
};