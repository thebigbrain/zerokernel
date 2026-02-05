#pragma once
#include <cstdint>

#include "SignalType.hpp"

/**
 * @brief 信号上下文：跨平台的现场快照
 * 无论是硬件触发还是 Mock 注入，都会生成此快照。
 */
class ISignalContext
{
public:
    virtual ~ISignalContext() = default;

    // 提供通用的现场访问接口，不暴露特定硬件寄存器
    virtual uintptr_t get_instruction_pointer() const = 0;
    virtual uintptr_t get_stack_pointer() const = 0;

    // 允许内核或测试框架修改现场（例如系统调用返回结果）
    virtual void set_return_value(uintptr_t value) = 0;
};

/**
 * @brief 统一信号包
 */
struct SignalPacket
{
    SignalType type;       // 信号大类 (语义层：怎么发生的？)
    SignalEvent event_id;  // 信号小类 (逻辑层：具体是什么事？)
    ISignalContext *frame; // 物理层现场
};

/**
 * @brief 内核信号监听契约
 * * 只有实现了此接口的实体，才能在 Root Task 运行期间
 * 接收来自硬件或 Mock 分发出的物理事件。
 */
class ISignalListener
{
public:
    virtual ~ISignalListener() = default;

    /**
     * @brief 信号到达时的唯一处理入口
     * * @param packet 信号包（包含类型：中断/异常/同步，以及向量号）
     * @param context 被打断一瞬间的上下文现场（寄存器快照）
     */
    virtual void on_signal_received(SignalPacket packet) = 0;
};

class ISignalDispatcher
{
public:
    virtual ~ISignalDispatcher() = default;

    /**
     * @brief 绑定内核监听器
     */
    virtual void bind_listener(ISignalListener *listener) = 0;

    /**
     * @brief 激活分发通道（开启全局中断/开启模拟时钟）
     */
    virtual void activate() = 0;

    /**
     * @brief 关闭分发通道（进入临界区保护时使用）
     */
    virtual void deactivate() = 0;
};
