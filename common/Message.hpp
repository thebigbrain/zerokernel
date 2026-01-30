#pragma once
#include <cstdint>

enum MessageType
{
    SYS_LOAD_TASK = 1,
    EVENT_KEYBOARD = 2,
    EVENT_PRINT = 0x100
};

struct Message
{
    MessageType type;
    uint64_t payload[4];
};