cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------
# Project setup
# ------------------------------------------------------------

set(CMAKE_PROJECT_NAME battery_management_system_26)

# Toolchain must be included before project()
include("cmake/gcc-arm-none-eabi.cmake")

project(${CMAKE_PROJECT_NAME} C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message("Build type: ${CMAKE_BUILD_TYPE}")

# ------------------------------------------------------------
# Target
# ------------------------------------------------------------

add_executable(${CMAKE_PROJECT_NAME})

# ------------------------------------------------------------
# Linker script and startup file
# ------------------------------------------------------------

set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32F446RETx_FLASH.ld")

set(STARTUP_SOURCES "")

if(EXISTS "${CMAKE_SOURCE_DIR}/startup_stm32f446xx.s")
    list(APPEND STARTUP_SOURCES "${CMAKE_SOURCE_DIR}/startup_stm32f446xx.s")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/Core/Startup/startup_stm32f446xx.s")
    list(APPEND STARTUP_SOURCES "${CMAKE_SOURCE_DIR}/Core/Startup/startup_stm32f446xx.s")
else()
    message(FATAL_ERROR "Could not find startup_stm32f446xx.s")
endif()

if(NOT EXISTS "${LINKER_SCRIPT}")
    message(FATAL_ERROR "Could not find linker script: ${LINKER_SCRIPT}")
endif()

# ------------------------------------------------------------
# Application sources
# ------------------------------------------------------------

file(GLOB MAIN_SOURCES
    "${CMAKE_SOURCE_DIR}/Core/Src/*.c"
    "${CMAKE_SOURCE_DIR}/Core/Src/adbms/*.c"
)

file(GLOB GOPHERCAN_SOURCES
    "${CMAKE_SOURCE_DIR}/../gophercan-lib/*.c"
)

file(GLOB GOPHERSENSE_SOURCES
    "${CMAKE_SOURCE_DIR}/../Gopher_Sense/lib/*.c"
    "${CMAKE_SOURCE_DIR}/../Gopher_Sense/Build/*.c"
)

# ------------------------------------------------------------
# STM32 HAL sources
# Only include real driver sources, not timebase template files.
# ------------------------------------------------------------

set(HAL_SOURCES
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_can.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
)

# Optional HAL files, depending on exact Cube version
foreach(OPTIONAL_HAL_SOURCE
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c"
)
    if(EXISTS "${OPTIONAL_HAL_SOURCE}")
        list(APPEND HAL_SOURCES "${OPTIONAL_HAL_SOURCE}")
    endif()
endforeach()

foreach(HAL_SOURCE ${HAL_SOURCES})
    if(NOT EXISTS "${HAL_SOURCE}")
        message(FATAL_ERROR "Missing HAL source: ${HAL_SOURCE}")
    endif()
endforeach()

# ------------------------------------------------------------
# FreeRTOS sources
# Core/Src/freertos.c is only CubeMX app glue.
# These are the actual kernel/CMSIS-RTOS sources.
# ------------------------------------------------------------

set(FREERTOS_SOURCES
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/croutine.c"
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c"
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/list.c"
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/queue.c"
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c"
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/tasks.c"
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/timers.c"
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c"
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c"
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c"
)

foreach(FREERTOS_SOURCE ${FREERTOS_SOURCES})
    if(NOT EXISTS "${FREERTOS_SOURCE}")
        message(FATAL_ERROR "Missing FreeRTOS source: ${FREERTOS_SOURCE}")
    endif()
endforeach()

# ------------------------------------------------------------
# Add all sources
# ------------------------------------------------------------

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${STARTUP_SOURCES}
    ${MAIN_SOURCES}
    ${GOPHERCAN_SOURCES}
    ${GOPHERSENSE_SOURCES}
    ${HAL_SOURCES}
    ${FREERTOS_SOURCES}
)

# ------------------------------------------------------------
# Include paths
# ------------------------------------------------------------

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/Core/Inc"

    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include"

    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include"
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS"
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F"

    "${CMAKE_SOURCE_DIR}/../gophercan-lib"
    "${CMAKE_SOURCE_DIR}/../Gopher_Sense/lib"
    "${CMAKE_SOURCE_DIR}/../Gopher_Sense/Build"
)

# ------------------------------------------------------------
# Defines
# ------------------------------------------------------------

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    USE_HAL_DRIVER
    STM32F446xx
    $<$<CONFIG:Debug>:DEBUG>
)

# ------------------------------------------------------------
# Compile options
# ------------------------------------------------------------

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    -mcpu=cortex-m4
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard

    -Wall
    -Wextra
    -Wpedantic
    -fdata-sections
    -ffunction-sections

    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-Os>
)

# ------------------------------------------------------------
# Link options
# ------------------------------------------------------------

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -u _printf_float
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    c
    m
)

# ------------------------------------------------------------
# Convert output to hex and binary
# ------------------------------------------------------------

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
)