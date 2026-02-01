#pragma once
#include <cstdint>

#define ZIMG_MAGIC 0xDEADBEEF

#pragma pack(push, 1) // 强制 1 字节对齐，必须与 Python 的 struct.pack 匹配

enum class SectionType : uint32_t
{
    ROOT_TASK = 1,
    DRIVER = 2,
    CONFIG = 3,
    DATA = 4
};

struct ZImgSection
{
    char name[8];            // 8 bytes
    uint32_t type;           // 4 bytes (SectionType)
    uint32_t file_offset;    // 4 bytes
    uint64_t dest_phys_addr; // 8 bytes
    uint32_t size;           // 4 bytes
}; // Total: 28 bytes

struct ZImgHeader
{
    uint32_t magic;           // 4 bytes
    uint32_t version;         // 4 bytes
    uint32_t header_size;     // 4 bytes
    uint32_t section_count;   // 4 bytes
    uint64_t root_entry_off;  // 8 bytes
    uint64_t config_phys;     // 8 bytes
    uint64_t memory_required; // 8 bytes
}; // Total: 44 bytes

#pragma pack(pop)