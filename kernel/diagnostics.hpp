#pragma once

#include <cstdint>

// 1. 枚举定义
enum class PRINT_LEVEL : uint8_t
{
    LEVEL_DEBUG = 0,
    LEVEL_INFO,
    LEVEL_WARN,
    LEVEL_ERROR,
    LEVEL_FATAL,
};

// 2. 外部接口声明
extern "C"
{
    void kernel_print_handler(const char *msg, PRINT_LEVEL level);
}

// 3. 基础打印宏：只负责调用 handler，不负责挂起
#ifndef K_PRINT_INTERNAL
#define K_PRINT_INTERNAL(msg, level)                           \
    do                                                         \
    {                                                          \
        kernel_print_handler(msg, PRINT_LEVEL::LEVEL_##level); \
    } while (0)
#endif

// 4. PANIC 宏：打印后进入无限循环（这是内核最后的不归路）
#ifndef K_PANIC
#define K_PANIC(msg)                  \
    do                                \
    {                                 \
        K_PRINT_INTERNAL(msg, FATAL); \
        for (;;)                      \
            ;                         \
    } while (0)
#endif

// 5. 日志宏：打印后正常返回，继续执行
#ifndef K_INFO
#define K_INFO(msg) K_PRINT_INTERNAL(msg, INFO)
#endif

#ifndef K_DEBUG
#define K_DEBUG(msg) K_PRINT_INTERNAL(msg, DEBUG)
#endif

#ifndef K_WARN
#define K_WARN(msg) K_PRINT_INTERNAL(msg, WARN)
#endif

#ifndef K_ERROR
#define K_ERROR(msg) K_PRINT_INTERNAL(msg, ERROR)
#endif

// 6. 断言宏：修复 __LINE__ 拼接导致的 C2143 错误
#ifndef K_ASSERT
#ifdef NDEBUG
#define K_ASSERT(condition, msg) ((void)0)
#else
#define K_ASSERT(condition, msg)                                        \
    do                                                                  \
    {                                                                   \
        if (!(condition))                                               \
        {                                                               \
            K_ERROR("ASSERT FAILED: " msg);                             \
            /* __LINE__ 是 int，不能直接拼在字符串后面 */               \
            /* 如果需要行号，建议在平台层的 print_handler 中自动处理 */ \
        }                                                               \
    } while (0)
#endif
#endif