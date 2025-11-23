cmake_minimum_required(VERSION 3.22)

add_library(DriversExt STATIC
    LCD/LT768Driver.cpp
)

target_include_directories(DriversExt PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/LCD_Drivers
    ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
)

target_link_libraries(DriversExt PUBLIC
    stm32cubemx
    LCD_Drivers
)
