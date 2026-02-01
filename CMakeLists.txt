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

# 定义统一的二进制分发目录
set(DIST_DIR "${CMAKE_BINARY_DIR}/dist")
file(MAKE_DIRECTORY ${DIST_DIR}) # 确保目录存在

set(FINAL_IMAGE "${CMAKE_BINARY_DIR}/OS_FULL_PHYSICAL.img")
set(ROOT_TASK_BIN "${DIST_DIR}/root_task.bin")
# --- 2. 添加各模块子目录 ---

# 内核层：生成 kernel.lib
add_subdirectory(kernel)

# 模拟器层：生成 simulator.exe (链接 kernel.lib)
# 模拟器作为宿主，负责加载镜像并启动内核
add_subdirectory(simulator)

# 应用层：生成各种 bin 并放入 dist
add_subdirectory(apps/root_task)
# add_subdirectory(apps/kbd_driver)

add_subdirectory(tests)

# --- 3. 镜像合成逻辑 (os_image) ---

# 查找 Python 解释器
find_package(Python3 REQUIRED)


# --- 镜像合成逻辑（放在最后） ---
# 这里我们显式列出需要打包进镜像的文件
set(BUILD_ZIMG_SCRIPT "${CMAKE_SOURCE_DIR}/tools/build_zimg.py")

add_custom_command(
    OUTPUT ${FINAL_IMAGE}
    COMMAND ${Python3_EXECUTABLE} ${BUILD_ZIMG_SCRIPT} 
            ${FINAL_IMAGE} 
            ${ROOT_TASK_BIN}
            # 这里可以继续追加其他 bin
            # ${KBD_BIN}
    DEPENDS prepare_root_task ${ROOT_TASK_BIN} ${BUILD_ZIMG_SCRIPT}
    COMMENT "Synthesizing ZeroKernel OS Image"
)

add_custom_target(os_image ALL DEPENDS ${FINAL_IMAGE})
