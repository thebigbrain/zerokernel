#include <windows.h>
#include <iostream>
#include "kernel/types.hpp"
#include "kernel/kernel.hpp"

extern "C" void context_switch_asm(void **old_sp, void *new_sp);

int main()
{
    // 1. 准备物理领土
    size_t mem_size = 64 * 1024 * 1024;
    void *phys_mem = VirtualAlloc(NULL, mem_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (!phys_mem)
        return -1;

    // 2. 填充 BootInfo 契约
    BootInfo info;
    info.memory_base = phys_mem;
    info.memory_size = mem_size;
    info.switcher = context_switch_asm;

    // 3. 在“物理内存”起始位置构造内核对象
    Kernel *kernel = new (phys_mem) Kernel(&info);

    std::cout << "[Bootloader] Handing over control to Kernel..." << std::endl;

    kernel->bootstrap();

    return 0;
}