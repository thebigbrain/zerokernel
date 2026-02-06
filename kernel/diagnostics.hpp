#pragma once

#include <cstdint>

// 1. 使用带有前缀的名称，防止被系统宏（如 Windows.h 里的 ERROR）污染
enum class PRINT_LEVEL : uint8_t
{
    LEVEL_DEBUG = 0,
    LEVEL_INFO,
    LEVEL_WARN,
    LEVEL_ERROR, // 避开了 ERROR 这个宏
    LEVEL_FATAL,
};

// 2. 外部接口声明
extern "C"
{
    void kernel_print_handler(const char *msg, PRINT_LEVEL level);
}

#ifndef K_PRINT
#define K_PRINT(msg, level)                                    \
    do                                                         \
    {                                                          \
        kernel_print_handler(msg, PRINT_LEVEL::LEVEL_##level); \
        for (;;)                                               \
            ; /* 确保 Panic 不会返回 */                        \
    } while (0)
#endif

// 3. 宏定义保持逻辑一致
#ifndef K_PANIC
#define K_PANIC(msg) \
    K_PRINT(msg, FATAL)
#endif

#ifndef K_INFO
#define K_INFO(msg) K_PRINT(msg, INFO)
#endif

#ifndef K_DEBUG
#define K_DEBUG(msg) K_PRINT(msg, DEBUG)
#endif

#ifndef K_WARN
#define K_WARN(msg) K_PRINT(msg, WARN)
#endif

#ifndef K_ERROR
#define K_ERROR(msg) K_PRINT(msg, ERROR)
#endif

// 定义断言宏
#ifndef K_ASSERT
#ifdef NDEBUG
// Release 模式下，断言通常被优化掉以保证性能
#define K_ASSERT(condition, msg) ((void)0)
#else
// Debug 模式下，触发详细报错并中断
#define K_ASSERT(condition, msg)                           \
    do                                                     \
    {                                                      \
        if (!(condition))                                  \
        {                                                  \
            K_ERROR("ASSERTION FAILED: " msg);             \
            K_DEBUG("File: " __FILE__ " Line: " __LINE__); \
        }                                                  \
    } while (0)
#endif
#endif