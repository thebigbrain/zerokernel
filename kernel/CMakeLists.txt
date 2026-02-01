add_library(kernel STATIC 
    Kernel.cpp 
    Kernel.hpp
    Task.hpp
    KernelProxy.hpp
    MessageBus.hpp
    ObjectFactory.hpp
    ObjectFactory.cpp
    KList.hpp
    KObjectPool.hpp
    Memory.hpp
)
target_include_directories(kernel PUBLIC ${CMAKE_SOURCE_DIR})