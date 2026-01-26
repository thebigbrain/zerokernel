cmake_minimum_required(VERSION 3.28)
project(ZeroKernel LANGUAGES CXX C ASM_MASM) # 必须启用 ASM_MASM

# 强制要求 x64 环境
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This kernel requires a 64-bit target.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    # 只为 C 和 C++ 编译器添加 /utf-8
    add_compile_options("$<$<AND:$<C_COMPILER_ID:MSVC>,$<NOT:$<COMPILE_LANGUAGE:ASM_MASM>>>:/utf-8>")
    add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<NOT:$<COMPILE_LANGUAGE:ASM_MASM>>>:/utf-8>")
endif()

# 包含路径
include_directories("src" "include")

# 源文件列表
set(KERNEL_SOURCES
    src/kernel/kernel.cpp
    src/kernel/kernel.hpp
    src/kernel/task.hpp
    src/kernel/ITaskContext.hpp
    src/kernel/ICPUEngine.hpp
    src/kernel/memory.hpp
)

set(BOOT_SOURCES
    src/boot/main.cpp
    src/boot/context_switch.asm # CMake 会自动识别并调用 ml64.exe
)

# 生成可执行程序
add_executable(KernelSim ${KERNEL_SOURCES} ${BOOT_SOURCES})

# Windows 特殊设置：关闭增量链接（防止干扰汇编地址）并设置栈大小
if(MSVC)
    set_target_properties(KernelSim PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")
endif()
