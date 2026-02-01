add_executable(root_task main.cpp)

# 修正编译器选项
target_compile_options(root_task PRIVATE 
    /GS-                        # 必须：禁用缓冲区安全检查
    /Od                         # 建议：Debug 模式下关闭优化以方便调试
    /Zl                         # 建议：忽略默认库名（类似 /NODEFAULTLIB 的编译版）
)

# 修正链接器选项
# 我们需要显式关闭 DYNAMICBASE，这样 /FIXED 才能生效
set_target_properties(root_task PROPERTIES 
    LINK_FLAGS "/ENTRY:_binary_root_task_entry /NODEFAULTLIB /SUBSYSTEM:CONSOLE /FIXED /DYNAMICBASE:NO"
)

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