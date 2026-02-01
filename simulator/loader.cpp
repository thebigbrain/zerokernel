// simulator/loader.cpp 或 main.cpp

#include <iostream>
#include <fstream>
#include <cstring>
#include <common/ZImg.hpp> // 包含之前定义的 ZImgHeader, ZImgSection
#include <kernel/Memory.hpp>
#include <common/BootInfo.hpp>

// 辅助函数：解析 PE/ELF 入口点偏移
uintptr_t get_entry_offset(void *buffer)
{
    uint8_t *raw = (uint8_t *)buffer;

    // 1. Windows PE (MZ...)
    if (raw[0] == 'M' && raw[1] == 'Z')
    {
        uint32_t pe_header_ptr = *(uint32_t *)(raw + 0x3C);
        // AddressOfEntryPoint 在可选头偏移 0x28 处
        uint32_t entry_point_rva = *(uint32_t *)(raw + pe_header_ptr + 0x28);
        return (uintptr_t)entry_point_rva;
    }

    // 2. Linux ELF (\x7F ELF)
    if (raw[0] == 0x7F && raw[1] == 'E' && raw[2] == 'L' && raw[3] == 'F')
    {
        // 假设是 64 位 ELF (Entry point 偏移 24)
        uint64_t entry_addr = *(uint64_t *)(raw + 24);
        // 注意：ELF 入口通常是绝对虚拟地址，需要剥离基址（这里假设基址是 0x400000）
        // 在裸机/简单环境下，通常需要链接脚本配合生成相对地址
        return (uintptr_t)(entry_addr & 0xFFFFFF);
    }

    // 3. 纯二进制
    return 0;
}

void load_os_image(const char *path, PhysicalMemoryLayout layout, BootInfo *info)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
    {
        std::cerr << "[Loader] Failed to open image: " << path << std::endl;
        return;
    }

    // 1. 读取头部
    ZImgHeader header;
    f.read((char *)&header, sizeof(ZImgHeader));
    if (header.magic != ZIMG_MAGIC)
    {
        std::cerr << "[Loader] Invalid image magic!" << std::endl;
        return;
    }

    // 2. 遍历段表
    for (uint32_t i = 0; i < header.section_count; ++i)
    {
        ZImgSection sec;
        f.read((char *)&sec, sizeof(ZImgSection));

        // 暂存段表位置，跳转到数据区
        std::streampos next_sec_ptr = f.tellg();
        f.seekg(sec.file_offset);

        // 计算物理目标地址
        void *target_phys_addr = (void *)((uintptr_t)layout.base + (uintptr_t)sec.dest_phys_addr);

        // 读取数据到模拟物理内存
        f.read((char *)target_phys_addr, sec.size);

        // 3. 核心逻辑：如果是 ROOT 段，解析入口点
        if (memcmp(sec.name, "ROOT", 4) == 0)
        {
            uintptr_t entry_offset = get_entry_offset(target_phys_addr);
            info->root_task_entry = (void (*)(void *, void *))((uintptr_t)target_phys_addr + entry_offset);

            std::cout << "[Loader] RootTask Entry set to: " << (void *)info->root_task_entry
                      << " (Offset: " << entry_offset << ")" << std::endl;
        }
        else if (memcmp(sec.name, "SYSCONF", 7) == 0)
        {
            info->config_ptr = target_phys_addr;
        }

        // 跳回到段表位置继续处理下一个
        f.seekg(next_sec_ptr);
    }

    info->magic = header.magic;
    info->memory_size = layout.size;
    f.close();
    std::cout << "[Loader] Image loaded successfully." << std::endl;
}