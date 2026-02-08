#pragma once

enum class SignalType
{
    Interrupt, // 异步脉冲：由外部物理世界或模拟时序产生（不可预测）
    Exception, // 运行故障：CPU 执行非法动作时被迫触发（错误现场）
    Yield,     // 同步请求：执行流主动发起的意图跳转（如系统调用）
    Directive  // 模拟指令：专门为 Mock/测试设计的控制信号（非物理存在）
};

enum class SignalEvent : uint32_t
{
    None,
    Timer,
    Keyboard,
    Mouse,
    Network,
    Disk,
    Power,
    Sleep,
    Wakeup,
    Reset,
    Halt,
    Reboot,
    Shutdown,
    Suspend,
    Resume,
    Pause,
    Yield = 0x71,
    Terminate = 0x72
};