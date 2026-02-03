#pragma once

#include "MessageCallback.hpp"

// 这个宏的作用是：自动生成一个静态的“跳板”并打包成 MessageCallback 对象
#define BIND_MESSAGE_CB(Class, Func, ObjPtr)                                     \
    MessageCallback(                                                             \
        [](const Message &m, void *ctx) { static_cast<Class *>(ctx)->Func(m); }, \
        static_cast<void *>(ObjPtr))