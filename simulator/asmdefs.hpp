#pragma once

extern "C" void context_switch_asm(void **old_sp, void *new_sp);
extern "C" void context_load_asm(void *sp);