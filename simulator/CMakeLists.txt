# enable_language(ASM_MASM)

add_library(simulator_common 
    WinCPUEngine.hpp
    context_switch.asm
    IdleTask.hpp
    loader.cpp
    Simulator.hpp
    WinTaskContext.hpp
    WinTaskContext.cpp
)

add_executable(simulator
    main.cpp 
)

target_link_libraries(simulator PRIVATE kernel simulator_common)

add_definitions(-DIMG_PATH="${FINAL_IMAGE}")