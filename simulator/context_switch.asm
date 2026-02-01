; Windows x64 Calling Convention
.code

; context_switch_asm(void **old_sp, void *new_sp)
; rcx = old_sp (存储旧栈顶的地址), rdx = new_sp (新栈顶的值)
context_switch_asm PROC
    ; --- 1. 保存内核现场 ---
    ; 此时 [rsp] 是返回地址 (对应结构体的 rip)
    ; 按照结构体从后往前的顺序 PUSH (栈向下增长)
    push rbp
    push rbx
    push rdi
    push rsi
    push r12
    push r13
    push r14
    push r15
    push r9
    push r8
    push rdx
    push rcx

    ; 此时内核栈的布局与 WinX64Regs 完全一致
    mov [rcx], rsp       ; 保存当前栈顶到 g_saved_kernel_sp

    ; --- 2. 切换到新任务 ---
    mov rsp, rdx         ; 载入 Root Task 的栈顶

    ; --- 3. 恢复新任务现场 ---
    ; 按照结构体从前往后的顺序 POP
    pop rcx
    pop rdx
    pop r8
    pop r9
    pop r15
    pop r14
    pop r13
    pop r12
    pop rsi
    pop rdi
    pop rbx
    pop rbp

    ; --- 4. 此时 RSP 指向 rip ---
    ret                  ; 弹出并跳转到任务入口
context_switch_asm ENDP

; context_load_asm(void* new_sp)
; rcx = new_sp (指向 WinX64Regs 结构体的起始地址)
context_load_asm PROC
    mov rsp, rcx         ; 切换栈指针到寄存器镜像区域

    ; 按照 WinX64Regs 结构体定义的顺序弹出
    ; 内存地址：低 -> 高
    ; 弹出顺序：rcx -> rdx -> r8 -> r9 -> r15... -> rbp -> rip
    pop rcx              ; 弹出第一个参数
    pop rdx              ; 弹出第二个参数
    pop r8               ; 弹出第三个参数
    pop r9               ; 弹出第四个参数
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop rsi
    pop rdi
    pop rbx
    pop rbp

    ; 此时 RSP 经过 12 次弹出，恰好指向结构体最后的 rip
    ret                  ; 弹出 rip 并跳转到任务入口 (Entry Point)
context_load_asm ENDP

END
