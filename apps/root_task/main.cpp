#include "common/IUserRuntime.hpp"
#include "common/RootTaskDef.hpp"

// 约定：内核将 Runtime 指针和配置指针通过寄存器传入
extern "C" void ROOT_TASK_ENTRY(IUserRuntime *rt)
{
    // 构造一条日志消息
    Message msg;
    msg.type = MessageType::EVENT_PRINT; // 约定 0x100 为调试日志消息

    // 我们可以把字符串的前 8 个字符强转进 payload（简单演示）
    // 或者指向一个约定的内存地址
    const char *hello = "HELLOOS";
    msg.payload[0] = *(uint64_t *)hello;

    // 通过注入的代理发送
    rt->publish(msg);

    while (true)
    {
        rt->yield();
    }
}