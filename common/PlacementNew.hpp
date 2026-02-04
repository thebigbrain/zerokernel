#pragma once

#include <cstddef>
#include <new> // 必须包含，以便引入标准库的 placement new 定义

/**
 * 针对 C++17 过度对齐对象的 Placement New 适配。
 * 只有当类使用了 alignas(16) 或更大对齐时，编译器才会寻找这个重载。
 * 我们只补全标准库可能缺失的对齐版本，不重定义标准版本。
 */

// 1. 适配 alignas(N) 对象的 placement new
// 注意：不要加 constexpr，也不要重新定义 void* 重载
inline void *operator new(std::size_t _Size, std::align_val_t _Al, void *_Ptr) noexcept
{
    (void)_Size;
    (void)_Al;
    return _Ptr;
}

// 2. 配套的 placement delete (用于异常路径的回退，符号必须存在)
inline void operator delete(void *_Block, std::align_val_t _Al, void *_Ptr) noexcept
{
    (void)_Block;
    (void)_Al;
    (void)_Ptr;
}
