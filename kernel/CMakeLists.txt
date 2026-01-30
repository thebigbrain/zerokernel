add_library(kernel STATIC 
    Kernel.cpp 
    Kernel.hpp
    Task.hpp
    MessageBus.hpp
    ObjectFactory.hpp
)
target_include_directories(kernel PUBLIC ${CMAKE_SOURCE_DIR})