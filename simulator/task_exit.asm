; Windows x64 Calling Convention
.code

; 这是一个通用的路由，不依赖具体的 Kernel 实现
[extern terminate_current_task_handler] ; 由 C++ 实现的包装函数

task_exit_router:
    ; 1. 屏蔽中断，防止在收割时发生切换
    cli
    
    ; 2. 调用 C++ 层的逻辑收割器
    ; 这里通常会通过全局指针获取当前的 ITaskManager 实例并调用
    call terminate_current_task_handler
    
    ; 3. 理论上永远不会执行到这里
    hlt

END