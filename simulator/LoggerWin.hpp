#pragma once

#include <cstdio> // 仅在此平台相关文件中包含
#include <cstdarg>
#include <iostream>

#include <common/diagnostics.hpp>

extern "C"
{
    void klog(LogLevel level, const char *fmt, ...)
    {
        const char *level_strs[] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

        char buffer[1024];
        va_list args;
        va_start(args, fmt);

        // 在 Windows 模拟器下，我们“借用”宿主机的 vsnprintf
        std::vsnprintf(buffer, sizeof(buffer), fmt, args);

        va_end(args);

        // 输出到控制台
        std::printf("[%s] %s\n", level_strs[(int)level], buffer);

        if (IsDebuggerPresent())
            __debugbreak();

        std::fflush(stdout);

        if (level == LogLevel::Fatal)
        {
            std::terminate(); // 确保不归路
        }
    }
}