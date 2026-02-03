#pragma once

#include "IObjectBuilder.hpp"

/**
 * GenericObjectBuilder: 通用的对象构建器实现
 */
class GenericObjectBuilder : public IObjectBuilder
{
public:
    using IObjectBuilder::IObjectBuilder; // 复用构造函数

    // 可以在这里增加内存监控、Trace 日志等逻辑
};