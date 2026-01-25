#pragma once

// 这是内核与 Bootloader 共同看到的硬件描述符
struct BootInfo
{
    void *memory_base;                 // 线性内存的起点 (VirtualAlloc 的返回值)
    size_t memory_size;                // 线性内存的总大小
    void (*switcher)(void **, void *); // 由 Bootloader 提供的汇编切换原语
};