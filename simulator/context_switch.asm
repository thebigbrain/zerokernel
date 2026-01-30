; Windows x64 Calling Convention
.code

; rcx = void** old_sp, rdx = void* new_sp
context_switch_asm PROC
    push rbp
    push rbx
    push rdi
    push rsi
    push r12
    push r13
    push r14
    push r15

    mov [rcx], rsp      ; 保存当前栈指针
    mov rsp, rdx        ; 切换到新栈指针

    pop r15
    pop r14
    pop r13
    pop r12
    pop rsi
    pop rdi
    pop rbx
    pop rbp
    ret
context_switch_asm ENDP

END