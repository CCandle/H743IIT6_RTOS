cmake_minimum_required(VERSION 3.22)

# ---- Collect sources with better organization ----
file(GLOB_RECURSE APP_C_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utils/*.c"
)

file(GLOB_RECURSE APP_CPP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utils/*.cpp"
)

list(REMOVE_ITEM APP_C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Utils/printf_c.c")

# 创建库并分别处理源文件
add_library(App STATIC)

# 分别添加C和C++源文件
target_sources(App PRIVATE
    ${APP_C_SOURCES}
    ${APP_CPP_SOURCES}
)

# 明确设置语言属性
set_source_files_properties(${APP_C_SOURCES} PROPERTIES LANGUAGE C)
set_source_files_properties(${APP_CPP_SOURCES} PROPERTIES LANGUAGE CXX)

# ---- Include directories ----
target_include_directories(App PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
)

# ---- Language specific settings ----
# C++ settings
target_compile_features(App PUBLIC cxx_std_23)
target_compile_options(App PRIVATE 
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

# C settings  
target_compile_options(App PRIVATE 
    $<$<COMPILE_LANGUAGE:C>:-std=gnu11>
)

# ---- Link CubeMX core ----
target_link_libraries(App PUBLIC
    stm32cubemx
    LCD_Drivers
    lvgl
)
