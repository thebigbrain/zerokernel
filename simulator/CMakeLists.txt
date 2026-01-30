enable_language(ASM_MASM)

add_executable(simulator 
    main.cpp 
    WinCPUEngine.hpp
    WinCPUEngine.cpp 
    context_switch.asm
    IdleTask.hpp
)

target_link_libraries(simulator PRIVATE kernel)