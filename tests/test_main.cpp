#include "test.hpp"
#include <iomanip> // 用于对齐字符串

// 定义颜色代码
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_RED "\033[1;31m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN "\033[1;36m"

int main()
{
    auto &registry = get_registry();

    std::cout << COLOR_CYAN << "===========================================" << COLOR_RESET << std::endl;
    std::cout << "    Kernel Infrastructure Test Suite       " << std::endl;
    std::cout << "    Total Tests Registered: " << registry.size() << std::endl;
    std::cout << COLOR_CYAN << "===========================================" << COLOR_RESET << std::endl;

    int passed = 0;
    int failed = 0;

    for (const auto &test : registry)
    {
        // 打印正在运行的状态，注意不加 std::endl
        // 使用 left 对齐，给测试名留出 40 个字符的空间
        std::cout << "[ RUN      ] " << std::left << std::setw(40) << test.name << std::flush;

        try
        {
            test.func();
            // \r 移回行首，重新覆盖打印 [  OK  ]
            std::cout << "\r" << COLOR_GREEN << "[       OK ] " << COLOR_RESET << std::left << std::setw(40) << test.name << std::endl;
            passed++;
        }
        catch (const std::exception &e)
        {
            std::cout << "\r" << COLOR_RED << "[  FAILED  ] " << COLOR_RESET << std::left << std::setw(40) << test.name
                      << " (" << COLOR_YELLOW << e.what() << COLOR_RESET << ")" << std::endl;
            failed++;
        }
        catch (...)
        {
            std::cout << "\r" << COLOR_RED << "[  FAILED  ] " << COLOR_RESET << std::left << std::setw(40) << test.name
                      << " (" << COLOR_YELLOW << "Unknown Crash" << COLOR_RESET << ")" << std::endl;
            failed++;
        }
    }

    std::cout << COLOR_CYAN << "===========================================" << COLOR_RESET << std::endl;
    if (failed == 0)
    {
        std::cout << COLOR_GREEN << "  ALL TESTS PASSED (" << passed << "/" << passed << ")" << COLOR_RESET << std::endl;
    }
    else
    {
        std::cout << COLOR_RED << "  TESTS FAILED: " << failed << " | Passed: " << passed << COLOR_RESET << std::endl;
    }

    return (failed == 0) ? 0 : 1;
}