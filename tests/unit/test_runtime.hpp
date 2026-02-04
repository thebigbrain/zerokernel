#pragma once

#include <iostream>
#include <mock/SimulatorMock.hpp>
#include <test_framework.hpp>

/**
 * Test 21: 验证 Runtime 接口注入与 VTable 存活性
 */

void test_runtime_interface_integration()
{
    std::cout << "[RUN      ] Runtime Interface Integration" << std::endl;

    // 1. 创建 Mock 运行环境
    MockUserRuntime *mock_rt = new MockUserRuntime();

    // 2. 模拟虚函数表校验
    // 在 C++ 中，对象的第一个 8 字节通常是 vptr
    void **vptr = *(void ***)mock_rt;
    K_ASSERT(vptr != nullptr, "VTable pointer (vptr) is NULL!");
    K_ASSERT(vptr[0] != nullptr, "First virtual function (publish) is NULL!");

    // 3. 模拟跳转前的寄存器准备
    MockRegisters regs;
    regs.rcx = (uint64_t)mock_rt; // 注入 rt 指针

    // 4. 执行一次模拟调用 (由内核代劳)
    // 模拟 RootTask 内部执行: rt->publish(msg)
    Message test_msg;
    test_msg.type = MessageType::EVENT_PRINT;
    const char *test_str = "TEST_OK";
    memcpy(&test_msg.payload[0], test_str, 8);

    // 通过注入的寄存器值找到对象并调用
    IUserRuntime *injected_rt = (IUserRuntime *)regs.rcx;
    injected_rt->publish(test_msg);

    // 5. 断言 Mock 成功捕获了消息
    K_ASSERT(mock_rt->sent_messages.size() > 0, "Runtime failed to capture message!");
    K_ASSERT(mock_rt->sent_messages[0].type == MessageType::EVENT_PRINT, "Message type mismatch!");

    delete mock_rt;
    std::cout << "  [PASS] Runtime injection and vtable call verified." << std::endl;
}
