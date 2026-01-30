add_executable(root_task main.cpp)

# 1. 修正编译器选项
target_compile_options(root_task PRIVATE 
    /GS-                        # 必须：禁用缓冲区安全检查
    /Od                         # 建议：Debug 模式下关闭优化以方便调试
    /Zl                         # 建议：忽略默认库名（类似 /NODEFAULTLIB 的编译版）
)

# 2. 修正链接器选项
# 我们需要显式关闭 DYNAMICBASE，这样 /FIXED 才能生效
set_target_properties(root_task PROPERTIES 
    LINK_FLAGS "/ENTRY:_binary_root_task_entry /NODEFAULTLIB /SUBSYSTEM:CONSOLE /FIXED /DYNAMICBASE:NO /ALIGN:16"
)

# 3. 针对 Debug 模式的额外清理
# 去掉 CMake 默认给 Debug 模式加的 /RTC1
if(MSVC)
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" PARENT_SCOPE)
endif()