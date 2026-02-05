#pragma once

// 声明包装函数
extern "C" [[noreturn]] void kernel_panic_handler(const char *msg);

#ifndef K_PANIC
#define K_PANIC(msg) kernel_panic_handler(msg)
#endif