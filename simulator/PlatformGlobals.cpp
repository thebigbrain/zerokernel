#include "kernel/ISchedulingControl.hpp"

// 这里是变量真正的定义处
// 库和测试工程都能通过链接此文件找到它
ISchedulingControl *g_platform_sched_ctrl = nullptr;
