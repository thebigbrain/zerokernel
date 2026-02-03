#pragma once
#include <cstdint>

enum class MessageType : uint32_t
{
    NONE = 0,
    SYS_LOAD_TASK = 1,
    EVENT_KEYBOARD = 2,
    EVENT_PRINT = 0x100
};

struct alignas(16) Message
{
    MessageType type;
    uint64_t payload[4];
};