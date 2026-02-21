#pragma once
#include <cstdint>

struct DisplayRegs
{
    uint32_t width;   // 当前宽度
    uint32_t height;  // 当前高度
    uint32_t pitch;   // 每行字节数 (Scanline width)
    uint32_t bpp;     // 每像素位数 (通常是 32)
    uint32_t status;  // 状态 (0: 准备就绪, 1: 正在刷新)
    uint32_t command; // 命令 (比如 1: 切换分辨率)
};
