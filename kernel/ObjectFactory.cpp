#include "ObjectFactory.hpp"

ObjectFactory::ObjectFactory(PhysicalMemoryLayout mem)
{
    uintptr_t base = reinterpret_cast<uintptr_t>(mem.base);
    uintptr_t aligned_base = (base + 15ULL) & ~15ULL;
    size_t padding = aligned_base - base;

    if (padding <= mem.size)
    {
        _current_p = reinterpret_cast<uint8_t *>(aligned_base);
        _remaining = mem.size - padding;
    }
    else
    {
        _current_p = nullptr;
        _remaining = 0;
    }
}

void *ObjectFactory::allocate_raw(size_t size)
{
    uintptr_t curr = reinterpret_cast<uintptr_t>(_current_p);

    // 1. 强制将返回给用户的指针对齐到 16 字节
    // 例如：0x...04 -> 0x...10
    uintptr_t aligned_addr = (curr + 15ULL) & ~15ULL;

    // 2. 计算填充占用的空间
    size_t padding = aligned_addr - curr;

    // 3. 对请求的大小也进行对齐，确保下一个分配的对象也是 16 字节对齐
    size_t aligned_size = (size + 15ULL) & ~15ULL;
    size_t total_needed = padding + aligned_size;

    if (_remaining < total_needed)
        return nullptr;

    _current_p = reinterpret_cast<uint8_t *>(aligned_addr + aligned_size);
    _remaining -= total_needed;

    return reinterpret_cast<void *>(aligned_addr);
}