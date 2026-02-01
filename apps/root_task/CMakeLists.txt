add_executable(root_task main.cpp)

# 修正编译器选项
target_compile_options(root_task PRIVATE 
    /GS-                        # 必须：禁用缓冲区安全检查
    /Od                         # 建议：Debug 模式下关闭优化以方便调试
    /Zl                         # 建议：忽略默认库名（类似 /NODEFAULTLIB 的编译版）
    /guard:cf-                  # 禁用控制流保护
)

# 修正链接器选项
if(MSVC)
    target_link_options(root_task PRIVATE 
        "/ENTRY:main"           # 强制指定入口函数名为 main
        "/NODEFAULTLIB"         # 禁用所有默认库 (非常重要!)
        "/SUBSYSTEM:NATIVE"     # 标记为原生系统程序
        "/ALIGN:16"             # 段对齐设为 16 字节
        "/DYNAMICBASE:NO"       # 禁用 ASLR（地址随机化）
        "/FIXED"                # 固定基址
    )
    
    # 由于禁用了默认库，你可能需要手动链接一些内核共用代码或实现简单的 memset/memcpy
    # target_link_libraries(root_task PRIVATE common_lib)
endif()

# 设置目标文件的输出名称和位置（可选，但提取 bin 更有用）
set(BIN_OUT "${ROOT_TASK_BIN}")

# 使用 OUTPUT 模式。这会让 CMake 意识到：要得到这个 .bin，必须运行此命令
add_custom_command(
    OUTPUT ${BIN_OUT}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:root_task> ${BIN_OUT}
    DEPENDS root_task
    COMMENT "Copying root_task to staging area: ${BIN_OUT}"
    VERBATIM
)

# 关键：必须有一个目标引用这个 OUTPUT，否则命令永远不会执行
# 我们创建一个伪目标，方便顶层 os_image 依赖
add_custom_target(prepare_root_task DEPENDS ${BIN_OUT})

# 针对 Debug 模式的额外清理
# 去掉 CMake 默认给 Debug 模式加的 /RTC1
if(MSVC)
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" PARENT_SCOPE)
endif()