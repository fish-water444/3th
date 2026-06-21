# BSP library: MCU peripherals, DJI motor driver, remote control
add_library(bsp STATIC)

target_sources(bsp PRIVATE
    ${CMAKE_SOURCE_DIR}/Bsp/MCU/bsp_CAN.c
    ${CMAKE_SOURCE_DIR}/Bsp/MCU/bsp_adc.c
    ${CMAKE_SOURCE_DIR}/Bsp/MCU/bsp_PWM.c
    ${CMAKE_SOURCE_DIR}/Bsp/MCU/bsp_i2c.c
    ${CMAKE_SOURCE_DIR}/Bsp/Motor/motor.c
    ${CMAKE_SOURCE_DIR}/Bsp/RemoteControl/remote_control.c
)
