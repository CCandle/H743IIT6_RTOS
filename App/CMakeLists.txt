cmake_minimum_required(VERSION 3.22)

set(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Shared include surface for all application components
set(APP_PLATFORM_INCLUDES
    ${APP_ROOT}
    ${APP_ROOT}/Config
    ${APP_ROOT}/Data
    ${APP_ROOT}/Drivers
    ${APP_ROOT}/IPC
    ${APP_ROOT}/Logic
    ${APP_ROOT}/Tasks
    ${APP_ROOT}/Test
    ${APP_ROOT}/Utils
    ${CMAKE_SOURCE_DIR}/OS
    ${CMAKE_SOURCE_DIR}/OS/Memory
    ${CMAKE_SOURCE_DIR}/DriversExt
    ${CMAKE_SOURCE_DIR}/DriversExt/LCD
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
)

add_library(app_platform INTERFACE)
target_include_directories(app_platform INTERFACE ${APP_PLATFORM_INCLUDES})
target_link_libraries(app_platform INTERFACE
    stm32cubemx
    OS
    DriversExt
    LCD_Drivers
    lvgl
)
target_compile_features(app_platform INTERFACE cxx_std_23)
target_compile_options(app_platform INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:C>:-std=gnu11>
)

# Utilities and common helpers
set(APP_UTILS_C_SOURCES ${APP_ROOT}/Utils/printf_c.c)
file(GLOB APP_UTILS_CPP_SOURCES "${APP_ROOT}/Utils/*.cpp")

add_library(app_utils STATIC
    ${APP_UTILS_C_SOURCES}
    ${APP_UTILS_CPP_SOURCES}
)
set_source_files_properties(${APP_UTILS_C_SOURCES} PROPERTIES LANGUAGE C)
target_include_directories(app_utils PUBLIC ${APP_ROOT}/Utils)
target_link_libraries(app_utils PUBLIC app_platform)

# Configuration surfaces
add_library(app_config INTERFACE)
target_include_directories(app_config INTERFACE ${APP_ROOT}/Config)
target_link_libraries(app_config INTERFACE app_platform)

# Data definitions
add_library(app_data INTERFACE)
target_include_directories(app_data INTERFACE ${APP_ROOT}/Data)
target_link_libraries(app_data INTERFACE app_platform app_config)

# Algorithmic logic
add_library(app_logic INTERFACE)
target_include_directories(app_logic INTERFACE ${APP_ROOT}/Logic)
target_link_libraries(app_logic INTERFACE app_platform app_data app_config)

# Services / IPC layer
file(GLOB_RECURSE APP_IPC_SOURCES "${APP_ROOT}/IPC/*.cpp")
add_library(app_ipc STATIC ${APP_IPC_SOURCES})
target_include_directories(app_ipc PUBLIC ${APP_ROOT}/IPC)
target_link_libraries(app_ipc PUBLIC app_platform app_data app_logic app_utils)

# Board-facing drivers
file(GLOB_RECURSE APP_DRIVER_SOURCES "${APP_ROOT}/Drivers/*.cpp")
add_library(app_drivers STATIC ${APP_DRIVER_SOURCES})
target_include_directories(app_drivers PUBLIC ${APP_ROOT}/Drivers)
target_link_libraries(app_drivers PUBLIC app_platform app_utils)

# RTOS tasks
file(GLOB_RECURSE APP_TASK_SOURCES "${APP_ROOT}/Tasks/*.cpp")
add_library(app_tasks STATIC ${APP_TASK_SOURCES})
target_include_directories(app_tasks PUBLIC ${APP_ROOT}/Tasks)
target_link_libraries(app_tasks PUBLIC
    app_platform
    app_ipc
    app_logic
    app_data
    app_drivers
    app_utils
    app_config
)

# Application bootstrap entry points
add_library(app_entry STATIC ${APP_ROOT}/init.cpp)
target_link_libraries(app_entry PUBLIC app_platform app_tasks app_drivers app_ipc app_utils)

# Internal test headers
add_library(app_test_support INTERFACE)
target_include_directories(app_test_support INTERFACE ${APP_ROOT}/Test)
target_link_libraries(app_test_support INTERFACE app_platform app_data app_logic)

# Public aggregation point consumed by the firmware image
add_library(App INTERFACE)
target_link_libraries(App INTERFACE
    app_platform
    app_utils
    app_config
    app_data
    app_logic
    app_ipc
    app_drivers
    app_tasks
    app_entry
    app_test_support
)
