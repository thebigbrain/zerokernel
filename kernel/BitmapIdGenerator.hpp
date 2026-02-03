#pragma once
#include "IIdGenerator.hpp"
#include <cstdint>
#include <cstring>

#include "KernelUtils.hpp"

/**
 * BitmapIdGenerator: 基于位图的 ID 分配器
 * 优点：空间利用率极高，状态跟踪直观
 */
template <size_t MAX_ID_COUNT = 64>
class BitmapIdGenerator : public IIdGenerator
{
private:
    // 每个 uint64_t 可以管理 64 个 ID
    static constexpr size_t WORD_COUNT = (MAX_ID_COUNT + 63) / 64;
    uint64_t _bitmap[WORD_COUNT];

public:
    BitmapIdGenerator()
    {
        memset(_bitmap, 0, sizeof(_bitmap));
        // 预留 ID 0，通常作为非法值或内核自身使用
        _bitmap[0] = 1;
    }

    uint32_t acquire() override
    {
        for (size_t i = 0; i < WORD_COUNT; ++i)
        {
            if (_bitmap[i] != 0xFFFFFFFFFFFFFFFF)
            {
                // 找到第一个为 0 的位 (x64 环境下可用编译器内建函数优化)
                int bit_pos = KernelUtils::Bit::find_first_set(_bitmap[i]);
                uint32_t id = static_cast<uint32_t>(i * 64 + bit_pos);

                if (id < MAX_ID_COUNT)
                {
                    _bitmap[i] |= (1ULL << bit_pos);
                    return id;
                }
            }
        }
        return 0; // 分配失败
    }

    void release(uint32_t id) override
    {
        if (id >= MAX_ID_COUNT)
            return;

        size_t word_idx = id / 64;
        int bit_pos = id % 64;

        // 语义非常明确：清除对应的位
        KernelUtils::Bit::clear(_bitmap[word_idx], bit_pos);
    }

    /**
     * 检查特定 ID 是否正在使用中
     */
    bool is_active(uint32_t id) const override
    {
        // 1. 边界检查：超出范围的 ID 显然不是 active
        if (id >= MAX_ID_COUNT)
        {
            return false;
        }

        // 2. 定位到具体的 word 和位偏移
        size_t word_idx = id / MAX_ID_COUNT;
        int bit_pos = id % MAX_ID_COUNT;

        // 3. 利用 KernelUtils 检查对应位是否被设为 1
        return KernelUtils::Bit::test(_bitmap[word_idx], bit_pos);
    }
};
