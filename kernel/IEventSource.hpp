#pragma once

#include <cstdint>

/**
 * @brief 硬件事件监听的统一契约
 * 它定义了物理信号如何“进入”内核的入口逻辑
 */
class IEventSource
{
public:
    virtual ~IEventSource() = default;

    /**
     * @brief 绑定监听回调
     * 当物理信号触发时，执行该 handler。
     * handler 内部通常负责保存上下文并进行任务切换。
     */
    using EventHandler = void (*)(uint32_t event_id, void *context);
    virtual void set_handler(EventHandler handler, void *context) = 0;

    /**
     * @brief 信号屏蔽控制 (原子性保证)
     */
    virtual void enable() = 0;
    virtual void disable() = 0;

    /**
     * @brief 检查信号挂起状态
     */
    virtual bool is_pending(uint32_t event_id) const = 0;
};

class IEventController
{
public:
    // 注册具体的硬件源
    virtual void register_source(uint32_t event_id, IEventSource *source) = 0;

    // 内核通过这个控制器来操作开关，而不是通过 Dispatcher
    virtual void enable_all() = 0;
    virtual void disable_all() = 0;
    virtual void set_mask(uint32_t event_id, bool enabled) = 0;
};