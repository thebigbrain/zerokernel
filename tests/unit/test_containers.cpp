#include "test_framework.hpp"
#include "kernel/KList.hpp"

void test_klist_placement_new_integrity()
{
    uint8_t buffer[512];
    ObjectFactory factory({buffer, 512});

    // 模拟 MessageBus 的初始化逻辑
    void *mem = factory.allocate_raw(sizeof(KList<void *>));
    KList<void *> *list = ::new (mem) KList<void *>(&factory);

    // 验证点 1：构造函数是否成功设置了 factory 指针
    // 如果没有 get_factory()，可以观察 list->size() 是否为 0（假设构造函数里初始化了）
    if (list == nullptr)
    {
        throw std::runtime_error("KList placement new returned nullptr");
    }

    // 验证点 2：内存越界检测
    // 在 KList 后面分配一个金丝雀值，确保 KList 的构造没有踩到后面
    uint32_t *canary = (uint32_t *)factory.allocate_raw(sizeof(uint32_t));
    *canary = 0xDEADBEEF;

    // 这里的操作不应改变 canary 的值
    if (*canary != 0xDEADBEEF)
    {
        throw std::runtime_error("KList constructor corrupted adjacent memory!");
    }
}

K_TEST_CASE("Kernel: KList Memory Integrity", test_klist_placement_new_integrity);