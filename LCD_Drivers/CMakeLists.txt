cmake_minimum_required(VERSION 3.22)

# ---- Collect sources with better organization ----
file(GLOB_RECURSE LCDDRIVER_C_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

file(GLOB_RECURSE LCDDRIVER_CPP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# 创建库并分别处理源文件
add_library(LCD_Drivers STATIC)

# 分别添加C和C++源文件
target_sources(LCD_Drivers PRIVATE
    ${LCDDRIVER_C_SOURCES}
    ${LCDDRIVER_CPP_SOURCES}
)

# 明确设置语言属性
set_source_files_properties(${LCDDRIVER_C_SOURCES} PROPERTIES LANGUAGE C)
set_source_files_properties(${LCDDRIVER_CPP_SOURCES} PROPERTIES LANGUAGE CXX)

# ---- Include directories ----
target_include_directories(LCD_Drivers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
)

# ---- Language specific settings ----
# C++ settings
target_compile_features(LCD_Drivers PUBLIC cxx_std_23)
target_compile_options(LCD_Drivers PRIVATE 
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

# C settings  
target_compile_options(LCD_Drivers PRIVATE 
    $<$<COMPILE_LANGUAGE:C>:-std=gnu11>
)

# ---- Link CubeMX core ----
target_link_libraries(LCD_Drivers PUBLIC
    stm32cubemx
    App
)
