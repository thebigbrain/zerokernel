; ==============================================================================
; Windows x64 Context Switching Module
; ==============================================================================

.code

; --- 语义化宏：保存寄存器上下文 ---
; 按照 WinX64Regs 结构体从后往前的顺序压栈
SAVE_WIN_X64_CONTEXT MACRO
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
ENDM

; --- 语义化宏：恢复寄存器上下文 ---
; 按照 WinX64Regs 结构体从前往后的顺序出栈
RESTORE_WIN_X64_CONTEXT MACRO
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
ENDM

; ------------------------------------------------------------------------------
; context_switch_asm(void **old_sp, void *new_sp)
; RCX = old_sp, RDX = new_sp
; ------------------------------------------------------------------------------
context_switch_asm PROC
    ; 1. 语义化保存：将当前内核/任务现场压入旧栈
    SAVE_WIN_X64_CONTEXT

    ; 2. 状态保存：将旧栈顶指针存入 old_sp 指向的内存
    mov [rcx], rsp

    ; 3. 语义化切换：载入新任务的栈指针
    mov rsp, rdx

    ; 4. 语义化恢复：从新栈弹出目标任务现场
    RESTORE_WIN_X64_CONTEXT

    pop rax         ; 弹出 entry_func 到 rax，此时 RSP 增加 8
    ; 此时 RSP 正好指向你 setup 时预留的 32 字节影子空间的起始位置
    
    jmp rax         ; 直接跳入任务，不再动 RSP
context_switch_asm ENDP

; ------------------------------------------------------------------------------
; context_load_asm(void* new_sp)
; RCX = new_sp
; ------------------------------------------------------------------------------
; context_load_asm PROC
;     ; 1. 语义化切换：直接抛弃当前栈，强行切换到 new_sp
;     mov rsp, rcx

;     ; 2. 语义化恢复：弹出预设的任务现场
;     RESTORE_WIN_X64_CONTEXT

;     ; 2. 此时 RSP 指向结构体里的 rip 成员
;     ; 我们把 rip 读入一个临时寄存器，并跳过它
;     pop rax         ; rax = entry_func, RSP 现在指向影子空间开头 (...FFE8)
    
;     ; 3. 【关键】补偿影子空间带来的偏移
;     ; 我们直接把 RSP 挪到影子空间的上方，紧贴着 exit_stub
;     add rsp, 32     ; 现在 RSP = ...0008 (指向 exit_stub)
    
;     ; 4. 跳转进入任务
;     jmp rax         ; 任务函数开始运行，它会自己向下开辟 32 字节使用
; context_load_asm ENDP

END