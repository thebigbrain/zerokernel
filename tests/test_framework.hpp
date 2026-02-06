#pragma once
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <functional>
#include <chrono>

typedef std::function<void()> TestFunc;

struct TestCase
{
    std::string name;
    TestFunc func;
};

// 使用 inline 允许在多个源文件中包含而不会导致重复定义 (C++17+)
// 或者在 .cpp 中定义它
inline std::vector<TestCase> &get_registry()
{
    static std::vector<TestCase> registry;
    return registry;
}

inline void register_test(const std::string &name, TestFunc func)
{
    get_registry().push_back({name, func});
}

// 宏定义：## 用于连接符，保证变量名唯一
// 使用 do-while(0) 结构可以增加宏的安全性
#define K_TEST_CASE(func, desc) \
    static bool func##_registered = []() { \
        register_test(desc, func); \
        return true; }()

// --- 稳健的内核断言宏 ---
#ifndef K_T_ASSERT
#define K_T_ASSERT(cond, msg)                                                           \
    do                                                                                  \
    {                                                                                   \
        if (!(cond))                                                                    \
        {                                                                               \
            std::ostringstream oss;                                                     \
            oss << msg;                                                                 \
            std::cerr << "\n[ASSERTION FAILED] " << __FILE__ << ":" << __LINE__ << "\n" \
                      << "Condition: " << #cond << "\n"                                 \
                      << "Message: " << oss.str() << std::endl;                         \
            throw std::runtime_error(oss.str());                                        \
        }                                                                               \
    } while (0)
#endif
