// simulator/loader.cpp 或 main.cpp

#include <iostream>
#include <fstream>
#include <cstring>
#include <common/ZImg.hpp> // 包含之前定义的 ZImgHeader, ZImgSection
#include <kernel/Memory.hpp>
#include <common/BootInfo.hpp>

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

        // --- 修正点 1：使用段定义的 dest_phys_addr，不要硬编码 ROOT_PHYS_ADDR ---
        uintptr_t target_pos = (uintptr_t)layout.base + (uintptr_t)sec.dest_phys_addr;
        void *target_phys_addr = (void *)target_pos;

        // 读取数据到模拟物理内存
        f.read((char *)target_phys_addr, sec.size);

        // 3. 核心逻辑处理
        if (sec.type == (uint32_t)SectionType::ROOT_TASK)
        {
            // --- 修正点 2：直接信任 Header 里的 root_entry_off ---
            // 注意：root_entry_off 是相对于 ROOT 段起始物理地址的偏移
            uintptr_t entry_addr = target_pos + header.root_entry_off;

            // --- 修正点 3：增加对齐校验 (必须是偶数，最好 16 字节对齐) ---
            if (entry_addr % 2 != 0)
            {
                std::cerr << "[Loader] FATAL: Misaligned Entry Point: " << (void *)entry_addr << std::endl;
                // 这里可以选择退出或报错
            }

            info->root_task_entry = (void (*)(void *, void *))entry_addr;

            uint8_t *code = (uint8_t *)info->root_task_entry;
            printf("[Loader] Instruction at Entry: %02X %02X %02X %02X\n",
                   code[0], code[1], code[2], code[3]);

            std::cout << "[Loader] RootTask Loaded at: " << (void *)target_pos
                      << " | Entry: " << (void *)info->root_task_entry
                      << " (Offset: " << header.root_entry_off << ")" << std::endl;
        }
        else if (sec.type == (uint32_t)SectionType::CONFIG)
        {
            info->config_ptr = target_phys_addr;
        }

        // 跳回到段表位置
        f.seekg(next_sec_ptr);
    }

    info->magic = header.magic;
    info->memory_size = layout.size;
    f.close();
    std::cout << "[Loader] Image loaded successfully." << std::endl;
}