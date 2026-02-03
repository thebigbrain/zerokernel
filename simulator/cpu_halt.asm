; Windows x64 / NASM 兼容语法
.code

public cpu_halt

cpu_halt proc
    ; 1. 屏蔽硬件中断
    ; 确保 CPU 进入 halt 状态后，不会被时钟或其他中断唤醒并继续执行后续指令
    cli
    
@@:
    ; 2. 使 CPU 进入低功耗挂起状态
    ; HLT 指令会停止指令执行，直到下一次（未屏蔽的）中断发生
    hlt
    
    ; 3. 这里的跳转是为了防止某些罕见的非屏蔽中断 (NMI) 唤醒 CPU 后跑飞
    jmp @b
cpu_halt endp

END