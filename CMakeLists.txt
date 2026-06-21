cmake_minimum_required(VERSION 3.20)
project(C_Board_Standard_Robot C ASM)

# Generate compile_commands.json for clangd / IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ====================================
# Global compile flags (CPU arch)
# ====================================
set(CPU_FLAGS_STR "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
separate_arguments(CPU_FLAGS UNIX_COMMAND "${CPU_FLAGS_STR}")

# ====================================
# Global preprocessor definitions
# ====================================
add_compile_definitions(
    USE_HAL_DRIVER
    STM32F407xx
    ARM_MATH_CM4
    ARM_MATH_MATRIX_CHECK
    ARM_MATH_ROUNDING
)

# ====================================
# Global include directories
# ====================================
include_directories(
    Core/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    Middlewares/Third_Party/FreeRTOS/Source/include
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
    Application/ChassisTask
    Bsp/MCU
    Bsp/Motor
    Bsp/RemoteControl
    Middlewares/Algorithm/Include
    Middlewares/Controller
    Middlewares/Filter/FirstOrderFilter
)

# ====================================
# Global compile options (per-language)
# ====================================
add_compile_options(
    ${CPU_FLAGS}
    -Wall
    -fdata-sections
    -ffunction-sections
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Debug>:-gdwarf-2>
    $<$<CONFIG:Debug>:-Og>
)

# ====================================
# Subdirectory libraries
# ====================================
add_subdirectory(Drivers)
add_subdirectory(Middlewares)
add_subdirectory(Bsp)
add_subdirectory(Core)
add_subdirectory(Application)

# ====================================
# Final executable
# ====================================
add_executable(${PROJECT_NAME}
    startup_stm32f407xx.s
)

# Link all libraries (--whole-archive ensures all symbols are kept for ISR vector table)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        -Wl,--whole-archive
        app
        core
        bsp
        middlewares
        drivers
        -Wl,--no-whole-archive
        -lc
        -lm
        -larm_cortexM4lf_math
)

# ====================================
# Linker options
# ====================================
target_link_options(${PROJECT_NAME} PRIVATE
    ${CPU_FLAGS}
    -specs=nano.specs
    -T${CMAKE_SOURCE_DIR}/STM32F407IGHx_FLASH.ld
    -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map,--cref
    -Wl,--gc-sections
    -L${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Lib/GCC
)

# ====================================
# Post-build: generate .hex and .bin, print size
# ====================================
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
            $<TARGET_FILE:${PROJECT_NAME}>
            ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S
            $<TARGET_FILE:${PROJECT_NAME}>
            ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "Generating .hex / .bin and printing size"
)
