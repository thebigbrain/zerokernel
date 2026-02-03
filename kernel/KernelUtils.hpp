#pragma once
#include <cstdint>
#include <cstddef>

// 必须放在全局作用域，不能放在函数里
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

/**
 * KernelUtils: 内核基础工具箱
 * 采用命名空间隔离，支持跨编译器硬件加速
 */
namespace KernelUtils
{

    /**
     * Bit: 提供位扫描、位测试等硬件加速操作
     */
    namespace Bit
    {
        static inline int find_first_set(uint64_t value)
        {
            if (value == 0)
                return -1;
#if defined(_MSC_VER) && !defined(__clang__)
            unsigned long index;
            return _BitScanForward64(&index, value) ? static_cast<int>(index) : -1;
#elif defined(__GNUC__) || defined(__clang__)
            return __builtin_ctzll(value);
#else
            int count = 0;
            while (!(value & 1))
            {
                value >>= 1;
                count++;
            }
            return count;
#endif
        }

        /**
         * 检查一个无符号整数是否为 2 的幂
         * 2 的幂在二进制中只有一个位是 1 (例如: 1, 2, 4, 8, 16...)
         */
        static inline bool is_power_of_two(uint64_t val)
        {
            // 0 不是 2 的幂
            // 原理：如果 n 是 2 的幂，则 n-1 的二进制表示是 n 的最高位之后全部取反
            // 例如：n = 8 (1000), n-1 = 7 (0111)。两者按位与 (&) 的结果必为 0
            return val > 0 && (val & (val - 1)) == 0;
        }

        static inline bool test(uint64_t value, int bit)
        {
            return (value & (1ULL << bit)) != 0;
        }

        static inline void set(uint64_t &value, int bit)
        {
            value |= (1ULL << bit);
        }

        static inline void clear(uint64_t &value, int bit)
        {
            value &= ~(1ULL << bit);
        }
    }

    /**
     * Align: 内存地址与数值对齐工具
     * 确保 x64 栈 (16-byte) 或 页表 (4KB) 对齐
     */
    namespace Align
    {
        template <typename T>
        static inline T up(T value, size_t alignment)
        {
            size_t a = (size_t)alignment;
            return (T)(((size_t)value + a - 1) & ~(a - 1));
        }

        template <typename T>
        static inline T down(T value, size_t alignment)
        {
            size_t a = (size_t)alignment;
            return (T)((size_t)value & ~(a - 1));
        }

        static inline bool is_aligned(size_t value, size_t alignment)
        {
            return (value & (alignment - 1)) == 0;
        }
    }

    /**
     * Memory: 基础内存操作（避免强制依赖标准库）
     */
    namespace Memory
    {
        static inline void copy(void *dest, const void *src, size_t n)
        {
            auto d = static_cast<uint8_t *>(dest);
            auto s = static_cast<const uint8_t *>(src);
            while (n--)
                *d++ = *s++;
        }

        static inline void zero(void *s, size_t n)
        {
            auto p = static_cast<uint8_t *>(s);
            while (n--)
                *p++ = 0;
        }
    }
}