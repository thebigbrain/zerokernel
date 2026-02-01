#pragma once
#include <common/ZImg.hpp>
#include <common/BootInfo.hpp>
#include <kernel/Memory.hpp>
#include "kernel/Kernel.hpp"

void load_os_image(const char *filename, PhysicalMemoryLayout layout, BootInfo *out_info);

// 模拟器内存初始化
PhysicalMemoryLayout setup_memory(size_t size)
{
    PhysicalMemoryLayout layout;
    layout.base = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    layout.size = size;
    return layout;
}