#include <cassert>
#include <stdexcept>
#include "kernel/ObjectFactory.hpp"

#include "test_framework.hpp"

void test_factory_out_of_memory()
{
    uint8_t buffer[64];
    PhysicalMemoryLayout layout{buffer, 64};
    ObjectFactory factory(layout);

    // 尝试分配超过 64 字节的内存
    void *p = factory.allocate_raw(128);

    // 契约验证：不应崩溃，应返回 nullptr
    if (p != nullptr)
    {
        throw std::runtime_error("Factory allocated beyond physical limits!");
    }
}

K_TEST_CASE("ObjectFactory Out of Memory", test_factory_out_of_memory);

void test_factory_alignment_integrity()
{
    size_t pool_size = 4096;
    void *pool = malloc(pool_size);
    PhysicalMemoryLayout layout{pool, pool_size};
    ObjectFactory factory(layout);

    // 强制请求一个奇数大小，观察下一次分配是否依然对齐
    factory.allocate_raw(13);
    void *p2 = factory.allocate_raw(8);

    uintptr_t addr = reinterpret_cast<uintptr_t>(p2);
    // 内核对象通常要求至少 8 或 16 字节对齐
    assert(addr % 8 == 0 && "Subsequent allocation must maintain alignment");

    free(pool);
}

void test_memory_corruption()
{
    PhysicalMemoryLayout layout{malloc(2048), 2048};
    ObjectFactory factory(layout);

    // 1. 分配第一个对象并填充已知特征码
    uint64_t *p1 = (uint64_t *)factory.allocate_raw(64);
    for (int i = 0; i < 8; ++i)
        p1[i] = 0xAAAAAAAAAAAAAAAA;

    // 2. 分配第二个对象
    uint64_t *p2 = (uint64_t *)factory.allocate_raw(64);
    for (int i = 0; i < 8; ++i)
        p2[i] = 0xBBBBBBBBBBBBBBBB;

    // 3. 验证第一个对象是否被破坏
    for (int i = 0; i < 8; ++i)
    {
        if (p1[i] != 0xAAAAAAAAAAAAAAAA)
        {
            throw std::runtime_error("Memory overlap detected: Object A corrupted by Object B");
        }
    }
}

void test_factory_allocation() {}

K_TEST_CASE("ObjectFactory Lifecycle", test_factory_allocation);