#pragma once

/**
 * 体系结构无关的上下文句柄
 * 内核只看到一个指针，具体内容由底层的汇编原语解释
 */

struct PhysicalMemoryLayout
{
    void *base;
    size_t size;
    // 内核利用这些信息，在上面“施工”，构建 Task 对象
};