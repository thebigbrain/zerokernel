// common/BootInfo.hpp
#pragma once
#include <cstdint>

#include "ZImg.hpp"

struct BootInfo
{
    uint32_t magic; // 用于校验，比如 0xDEADBEEF
    uint32_t version;

    // 约定的入口指针
    void (*root_task_entry)(void *rt, void *arg);
    void (*idle_task_entry)(void *arg);

    // 约定的资源位置
    void *config_ptr;
    uint64_t memory_size;

    uint32_t extra_sections_count;
    ZImgSection *sections_table; // 指向加载到内存中的段表镜像
};