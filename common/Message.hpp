#pragma once
#include <cstdint>

enum class MessageType : uint32_t
{
    NONE = 0,
    SYS_LOAD_TASK = 1,
    KERNEL_EVENT = 0x10,
    EVENT_KEYBOARD = 0x100,
    EVENT_PRINT = 0x101
};

struct alignas(16) Message
{
    MessageType type;
    uint64_t payload[4];
};