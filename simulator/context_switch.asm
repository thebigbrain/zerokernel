; ==============================================================================
; Windows x64 Context Switching Module (ABI Compliant)
; ==============================================================================

.code

; --- 调整后的寄存器保存宏 ---
; 注意：在 Windows x64 中，有些寄存器是 Non-volatile (非易失) 的，
; 只需要保存：RBP, RBX, RDI, RSI, R12-R15。
; 但为了安全或调试，保存全部也是可以的，只要和 C++ 结构体完全对应。
SAVE_WIN_X64_CONTEXT MACRO
    push r15   ; 地址最高 (WinX64Regs + 0x58)
    push r14
    push r13
    push r12
    push rsi
    push rdi
    push rbx
    push rbp
    push r9
    push r8
    push rdx
    push rcx   ; 地址最低 (WinX64Regs + 0x00)，RSP 指向这里
ENDM

RESTORE_WIN_X64_CONTEXT MACRO
    pop rcx    ; 从偏移 0 弹出
    pop rdx
    pop r8
    pop r9
    pop rbp
    pop rbx
    pop rdi
    pop rsi
    pop r12
    pop r13
    pop r14
    pop r15
ENDM

; context_switch_asm(old_sp, new_sp)
context_switch_asm PROC
    SAVE_WIN_X64_CONTEXT
    mov [rcx], rsp
    mov rsp, rdx
    RESTORE_WIN_X64_CONTEXT
    ret                    ; 弹出 entry_func 并跳转
context_switch_asm ENDP

END