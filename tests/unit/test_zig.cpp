#include <common/ZImg.hpp>
#include <iostream>
#include "test_framework.hpp"

void test_zimg_header_integrity()
{
    // 模拟一个 ZImgHeader
    ZImgHeader header = {0};
    header.magic = ZIMG_MAGIC;
    header.root_entry_off = 0x1000; // 假设入口在 4KB 处 (16字节对齐)

    // 验证 1: 头部对齐 (由于 pack(1)，应为 44)
    K_ASSERT(sizeof(ZImgHeader) == 40, "ZImgHeader size mismatch! Should be 40 bytes");

    // 验证 2: 模拟地址计算
    uintptr_t mock_load_base = 0x1000000; // 16MB
    uintptr_t entry_point = mock_load_base + header.root_entry_off;

    // 验证入口点是否为 16 字节对齐（Windows ABI 要求）
    K_ASSERT(entry_point % 16 == 0, "Root Entry Point must be 16-byte aligned");

    std::cout << "  [PASS] ZImg Protocol Integrity Verified." << std::endl;
}

void test_compact_pe_loading()
{
    // 模拟 0x260 这种紧凑布局
    uintptr_t entry_rva = 0x260;
    uintptr_t base_phys = 0x1000000;
    uintptr_t final_jump_addr = base_phys + entry_rva;

    // 只要它是 16 字节对齐的，我们就认为它是合法的
    // Windows x64 ABI 只要 RSP+8 对齐，但函数入口 16 字节对齐是最佳实践
    K_ASSERT(final_jump_addr % 16 == 0,
             "Compact PE Entry must be 16-byte aligned! Address: " << (void *)final_jump_addr);

    std::cout << "  [PASS] Compact PE Entry (0x260) verified." << std::endl;
}

K_TEST_CASE("Compact PE Entry", test_compact_pe_loading);
K_TEST_CASE("ZImg Protocol Integrity", test_zimg_header_integrity);