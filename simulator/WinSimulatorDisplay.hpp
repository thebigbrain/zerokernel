#pragma once

#include <cstdint>

class WinSimulatorDisplay
{
    uint32_t *_framebuffer; // 模拟显存
    int _width = 800;
    int _height = 600;

public:
    void init()
    {
        // 1. 创建 Windows 窗口 (使用 GDI 或直接写位图)
        // 2. 分配 _framebuffer 内存 (800 * 600 * 4 字节)
    }

    uint32_t *get_physical_address() { return _framebuffer; }

    // 窗口循环：不断将 _framebuffer 的内容 StretchDIBits 到窗口上
    void refresh_loop();
};