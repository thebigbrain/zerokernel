#pragma once

#include <cstdint>

/**
 * IIdGenerator: 标识符生成器接口
 */
class IIdGenerator
{
public:
    virtual ~IIdGenerator() = default;

    // 获取一个唯一的 ID
    virtual uint32_t acquire() = 0;

    // 释放并回收 ID
    virtual void release(uint32_t id) = 0;

    // 检查 ID 是否正在使用 (可选，用于调试)
    virtual bool is_active(uint32_t id) const = 0;
};