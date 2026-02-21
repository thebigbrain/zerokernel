#include "common/IUserRuntime.hpp"

#include "draw.hpp"
#include <common/DisplayRegs.hpp>

// 约定：内核将 Runtime 指针和配置指针通过寄存器传入
extern "C" void main(IUserRuntime *rt)
{
    // 1. 索要硬件资源地址
    auto *regs = (DisplayRegs *)get_hw_addr(rt, "DISPLAY_REGS");
    auto *vram = (uint32_t *)get_hw_addr(rt, "DISPLAY_LFB");

    if (regs && vram)
    {
        // 清屏
        for (uint32_t i = 0; i < regs->width * regs->height; i++)
            vram[i] = 0x000000;

        // 打印系统看板
        const char *banner =
            "#######################################\n"
            "#  GEMINI OS MICROKERNEL v0.1         #\n"
            "#  Status: RUNNING (Simulation Mode)  #\n"
            "#######################################\n\n"
            "[  OK  ] Display Regs at 0x8000\n"
            "[  OK  ] LFB Mapping successful\n"
            "[ INFO ] Driver: Generic VGA 8x16\n"
            "[ INFO ] Resolution: 800x600\n";

        drive_draw_string(vram, banner, 20, 20, 0x00FF00, regs);
    }

    while (true)
        rt->yield();
}