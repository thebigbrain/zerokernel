cmake_minimum_required(VERSION 3.28)
project(ZeroKernel LANGUAGES CXX C ASM_MASM) # 必须启用 ASM_MASM

# 强制要求 x64 环境
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This kernel requires a 64-bit target.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含路径
include_directories("src" "include")

# 源文件列表
set(KERNEL_SOURCES
    src/kernel/kernel.cpp
    src/kernel/kernel.hpp
    src/kernel/task.hpp
    src/kernel/types.hpp
)

set(BOOT_SOURCES
    src/boot/main_sim.cpp
    src/boot/context_switch.asm # CMake 会自动识别并调用 ml64.exe
)

# 生成可执行程序
add_executable(KernelSim ${KERNEL_SOURCES} ${BOOT_SOURCES})

# Windows 特殊设置：关闭增量链接（防止干扰汇编地址）并设置栈大小
if(MSVC)
    set_target_properties(KernelSim PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")
    # 添加UTF-8编码支持
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    
    # 或者针对特定目标
    # target_compile_options(your_target PRIVATE /utf-8)
endif()
