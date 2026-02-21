#include <cstdint>
#pragma

struct HardwareResource
{
    uintptr_t base_address;
    size_t size;
    uint32_t type; // 比如：0-内存, 1-显存, 2-IO端口
};