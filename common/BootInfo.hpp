// common/BootInfo.hpp
#pragma once
#include <cstdint>

struct BootInfo
{
    uint32_t magic; // 用于校验，比如 0xDEADBEEF
    uint32_t version;

    // 约定的入口指针
    void (*root_task_entry)(void *rt, void *config);
    void (*idle_task_entry)(void *rt);

    // 约定的资源位置
    void *config_ptr;
    uint64_t memory_size;
};