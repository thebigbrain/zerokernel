#pragma once

#ifndef ROOT_TASK_ENTRY
#define ROOT_TASK_ENTRY _binary_root_task_entry
#endif

#ifndef CONFIG_AREA_ADDRESS
// 假设约定 32MB 处为配置区
#define CONFIG_AREA_ADDRESS 0x2000000
#endif
