# Drivers library: STM32F4xx HAL Driver (only the modules actually used)
add_library(drivers STATIC)

set(HAL_SRC_DIR ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src)

target_sources(drivers PRIVATE
    ${HAL_SRC_DIR}/stm32f4xx_hal.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_adc.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_adc_ex.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_can.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_cortex.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_dma.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_dma_ex.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_exti.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_flash.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_flash_ex.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_flash_ramfunc.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_gpio.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_i2c.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_i2c_ex.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_iwdg.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_pwr.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_pwr_ex.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_rcc.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_rcc_ex.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_spi.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_tim.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_tim_ex.c
    ${HAL_SRC_DIR}/stm32f4xx_hal_uart.c
)
