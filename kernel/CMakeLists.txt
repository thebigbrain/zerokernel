# 1. 搜寻当前目录下所有的源文件 (.cpp)
file(GLOB KERNEL_SOURCES CONFIGURE_DEPENDS "*.cpp")

# 2. 搜寻当前目录下所有的头文件 (.hpp)，确保它们包含在 IDE 的工程树中
file(GLOB KERNEL_HEADERS CONFIGURE_DEPENDS "*.hpp")

# 3. 定义静态库
add_library(kernel STATIC 
    ${KERNEL_SOURCES}
    ${KERNEL_HEADERS}
)

# 4. 设置头文件搜索路径
# PUBLIC 确保链接 kernel 库的 target 也能找到这些头文件
target_include_directories(kernel PUBLIC ${CMAKE_SOURCE_DIR})

# 5. (可选) 如果你的 kernel 依赖 common 目录，可以在这里添加
# target_link_libraries(kernel PRIVATE common)