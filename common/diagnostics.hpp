#pragma once

#include <cstdint>
#include <cstdarg>

enum class LogLevel
{
    Debug = 0,
    Info,
    Warn,
    Error,
    Fatal
};

// 内核日志入口

// 2. 外部接口声明
extern "C"
{
    void klog(LogLevel level, const char *fmt, ...);
}

// 3. 基础打印宏：只负责调用 handler，不负责挂起
#ifndef K_PRINT_INTERNAL
#define K_PRINT_INTERNAL(level, fmt, ...) \
    do                                    \
    {                                     \
        klog(level, fmt, ##__VA_ARGS__);  \
    } while (0)
#endif

// 4. PANIC 宏：打印后进入无限循环（这是内核最后的不归路）
#ifndef K_PANIC
#define K_PANIC(fmt, ...) \
    K_PRINT_INTERNAL(LogLevel::Fatal, fmt, ##__VA_ARGS__)
#endif

// 5. 日志宏：打印后正常返回，继续执行
#ifndef K_INFO
#define K_INFO(fmt, ...) K_PRINT_INTERNAL(LogLevel::Info, fmt, ##__VA_ARGS__)
#endif

#ifndef K_DEBUG
#define K_DEBUG(fmt, ...) K_PRINT_INTERNAL(LogLevel::Debug, fmt, ##__VA_ARGS__)
#endif

#ifndef K_WARN
#define K_WARN(fmt, ...) K_PRINT_INTERNAL(LogLevel::Warn, fmt, ##__VA_ARGS__)
#endif

#ifndef K_ERROR
#define K_ERROR(fmt, ...) K_PRINT_INTERNAL(LogLevel::Error, fmt, ##__VA_ARGS__)
#endif

// 6. 断言宏：修复 __LINE__ 拼接导致的 C2143 错误
#ifndef K_ASSERT
#ifdef NDEBUG
#define K_ASSERT(condition, fmt) ((void)0)
#else
#define K_ASSERT(condition, fmt, ...)                                   \
    do                                                                  \
    {                                                                   \
        if (!(condition))                                               \
        {                                                               \
            K_ERROR("ASSERT FAILED: " fmt, ##__VA_ARGS__);              \
            /* __LINE__ 是 int，不能直接拼在字符串后面 */               \
            /* 如果需要行号，建议在平台层的 print_handler 中自动处理 */ \
        }                                                               \
    } while (0)
#endif
#endif