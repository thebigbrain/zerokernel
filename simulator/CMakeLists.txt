# enable_language(ASM_MASM)
file(GLOB SIM_SOURCES CONFIGURE_DEPENDS "*.hpp" "*.cpp" "*.asm")

# 2. 从列表中移除 main.cpp
# 注意：路径需要匹配，如果 main.cpp 在子目录下，请使用完整相对路径
list(REMOVE_ITEM SIM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

add_library(simulator_common 
    ${SIM_SOURCES}
)

add_executable(simulator
    main.cpp 
)

target_link_libraries(simulator PRIVATE kernel simulator_common)

add_definitions(-DIMG_PATH="${FINAL_IMAGE}")