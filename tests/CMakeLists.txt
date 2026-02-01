# 自动搜索所有测试源文件
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "unit/*.cpp" "integration/*.cpp")

add_executable(kernel_tests test_main.cpp ${TEST_SOURCES})
target_link_libraries(kernel_tests PRIVATE kernel simulator_common) # 链接你的内核静态库

target_include_directories(kernel_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/tests
)

# 方便运行测试的命令
add_test(NAME AllKernelTests COMMAND kernel_tests)