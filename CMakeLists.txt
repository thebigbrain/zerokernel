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
    # add_compile_options(/fsanitize=address)
else()
    # add_compile_options(-fsanitize=address)
endif()

include_directories(${CMAKE_SOURCE_DIR})

# 1. 构建所有组件
add_subdirectory(apps/root_task)
# add_subdirectory(apps/kbd_driver)
add_subdirectory(kernel)

add_subdirectory(simulator)

# 查找 Python 解释器
find_package(Python3 REQUIRED)

add_custom_target(os_image ALL
    # 使用 Python3_EXECUTABLE 确保路径正确
    # 增加 WORKING_DIRECTORY 确保脚本能找到各个二进制文件
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/scripts/pack_image.py"
    DEPENDS kernel root_task
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "正在合成物理镜像 OS_FULL_PHYSICAL.img..."
    VERBATIM
)
