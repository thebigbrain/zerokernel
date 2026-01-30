add_library(kernel STATIC 
    PlacementNew.hpp
    Kernel.cpp 
    Kernel.hpp
    Task.hpp
    MessageBus.hpp
    ObjectFactory.hpp
)
target_include_directories(kernel PUBLIC ${CMAKE_SOURCE_DIR})