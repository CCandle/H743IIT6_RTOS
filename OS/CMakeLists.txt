cmake_minimum_required(VERSION 3.22)

add_library(OS STATIC
    Memory/memory.cpp
)

target_include_directories(OS PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Memory
)

target_link_libraries(OS PUBLIC
    stm32cubemx
)
