#pragma once
#include <common/ZImg.hpp>
#include <common/BootInfo.hpp>
#include <kernel/Memory.hpp>
#include "kernel/Kernel.hpp"

#include "IdleTask.hpp"
#include "WinTaskContextFactory.hpp"
#include "Win32SignalGate.hpp"
#include "Win32SchedulingControl.hpp"
#include <kernel/PlatformHooks.hpp>
#include "LoggerWin.hpp"
#include <thread>
#include <common/DisplayRegs.hpp>

extern "C" void kmain(PhysicalMemoryLayout layout,
                      BootInfo info,
                      PlatformHooks *platform_hooks);

extern ISchedulingControl *g_platform_sched_ctrl;

void load_os_image(const char *filename, PhysicalMemoryLayout layout, BootInfo *out_info);

// 模拟器内存初始化
PhysicalMemoryLayout setup_memory(size_t size)
{
    PhysicalMemoryLayout layout;
    layout.base = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    layout.size = size;
    return layout;
}

// 模拟物理显存
const int VRAM_WIDTH = 1080;
const int VRAM_HEIGHT = 720;
DisplayRegs g_gpu_regs = {VRAM_WIDTH, VRAM_HEIGHT, VRAM_WIDTH * 4, 32, 0, 0};
uint32_t g_physical_vram[VRAM_WIDTH * VRAM_HEIGHT];

// Win32 窗口刷新逻辑 (简化版)
void Win32_RefreshDisplay(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // 核心：使用 StretchDIBits 将数组直接画到窗口
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = VRAM_WIDTH;
    bmi.bmiHeader.biHeight = -VRAM_HEIGHT; // 自上而下
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    StretchDIBits(hdc, 0, 0, VRAM_WIDTH, VRAM_HEIGHT,
                  0, 0, VRAM_WIDTH, VRAM_HEIGHT,
                  g_physical_vram, &bmi, DIB_RGB_COLORS, SRCCOPY);

    EndPaint(hwnd, &ps);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
        Win32_RefreshDisplay(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    // 定时器强制刷新，确保即便内核不主动请求，窗口也会定期更新
    case WM_TIMER:
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// 窗口类名
const char *G_WND_CLASS = "Gemini_OS_Monitor";
HWND g_hMonitorWnd = NULL;

void MyWin32Refresh()
{
    if (g_hMonitorWnd)
    {
        // 这是真实的 Win32 API
        InvalidateRect(g_hMonitorWnd, NULL, FALSE);
    }
}

void run_simulator()
{
    // --- 初始化模拟内存 ---
    PhysicalMemoryLayout layout = setup_memory(128 * 1024 * 1024);
    BootInfo info;
    load_os_image(IMG_PATH, layout, &info);

    // 创建并填充资源账本
    static ResourceManager res_manager;

    // 注册显示器控制寄存器 (模拟 MMIO 空间)
    res_manager.register_hw("DISPLAY_REGS", (uintptr_t)&g_gpu_regs, sizeof(g_gpu_regs));

    // 注册线性显存
    res_manager.register_hw("DISPLAY_LFB", (uintptr_t)g_physical_vram, sizeof(g_physical_vram));

    // 以后加硬件只需加一行：res_manager.register_hw("KEYBOARD", 0x60, 1);

    // ---  创建内核线程 ---
    // 我们将内核逻辑封装在一个 lambda 或 std::thread 中
    std::thread kernel_thread([&]()
                              {
        auto* signal_dispatcher = new Win32SignalGate();
        auto* sched_control = new Win32SchedulingControl(signal_dispatcher);
        g_platform_sched_ctrl = sched_control;

        PlatformHooks hooks;
        hooks.dispatcher = signal_dispatcher;
        hooks.sched_control = sched_control;
        hooks.task_context_factory = new WinTaskContextFactory();
        hooks.halt = []() { Sleep(10); }; // 模拟时钟挂起
        hooks.refresh_display = MyWin32Refresh;
        hooks.resource_manager = &res_manager;

        // 进入 kmain，这会启动 RootTask
        kmain(layout, info, &hooks); });
    kernel_thread.detach(); // 让内核独立运行

    // ---  宿主主线程：创建 Win32 窗口 ---
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0,
                     hInstance, NULL, LoadCursor(NULL, IDC_ARROW),
                     (HBRUSH)(COLOR_WINDOW + 1), NULL, G_WND_CLASS, NULL};
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(G_WND_CLASS, "Gemini OS Monitor", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, VRAM_WIDTH + 16, VRAM_HEIGHT + 39,
                             NULL, NULL, hInstance, NULL);
    g_hMonitorWnd = hwnd;

    ShowWindow(hwnd, SW_SHOW);
    SetTimer(hwnd, 1, 33, NULL); // 设置约 30FPS 的自动刷新定时器

    // ---  宿主消息循环 ---
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}