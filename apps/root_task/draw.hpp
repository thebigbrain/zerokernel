#pragma once

#include <cstdint>
#include <common/IUserRuntime.hpp>
#include <common/DisplayRegs.hpp>

#include "font.hpp"

const unsigned char *get_font_bitmap(char c)
{
    // 1. 查找大写、数字与基础符号 (g_font_table)
    for (const auto &item : g_font_table)
    {
        if (item.ascii == c)
            return item.data;
    }

    // 4. 如果都没找到，返回空格的点阵 (通常是全 0)
    return g_font_table[0].data;
}

void drive_draw_string(uint32_t *vram, const char *str, int x, int y, uint32_t color, DisplayRegs *regs)
{
    int cur_x = x;
    int cur_y = y;
    uint32_t pitch_pixels = regs->pitch / 4;
    uint32_t screen_w = regs->width;
    uint32_t screen_h = regs->height;

    while (*str)
    {
        // 处理换行符
        if (*str == '\n')
        {
            cur_x = x;   // 回到起始列
            cur_y += 16; // 下移一行 (8x16 字体)
            str++;
            continue;
        }

        // 自动换行 (防止超出屏幕右边界)
        if (cur_x + 8 > (int)screen_w)
        {
            cur_x = x;
            cur_y += 16;
        }

        // 垂直越界检查
        if (cur_y + 16 > (int)screen_h)
            break;

        // 渲染字符
        const unsigned char *bitmap = get_font_bitmap(*str);
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (bitmap[i] & (0x80 >> j))
                {
                    vram[(cur_y + i) * pitch_pixels + (cur_x + j)] = color;
                }
            }
        }

        cur_x += 8;
        str++;
    }
}

uintptr_t get_hw_addr(IUserRuntime *rt, const char *name)
{
    uintptr_t addr = 0;
    Message m;
    m.type = MessageType::REQUEST_HARDWARE_INFO;
    m.payload[0] = (uintptr_t)name;
    m.payload[1] = (uintptr_t)&addr;
    rt->publish(m);
    return addr;
}