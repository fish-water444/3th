# Middlewares library: FreeRTOS + custom middleware (Controller, Algorithm, Filter)
add_library(middlewares STATIC)

set(FREERTOS_DIR ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source)

# FreeRTOS kernel
target_sources(middlewares PRIVATE
    ${FREERTOS_DIR}/croutine.c
    ${FREERTOS_DIR}/event_groups.c
    ${FREERTOS_DIR}/list.c
    ${FREERTOS_DIR}/queue.c
    ${FREERTOS_DIR}/stream_buffer.c
    ${FREERTOS_DIR}/tasks.c
    ${FREERTOS_DIR}/timers.c
    # FreeRTOS CMSIS-RTOS wrapper
    ${FREERTOS_DIR}/CMSIS_RTOS/cmsis_os.c
    # FreeRTOS port (GCC + ARM Cortex-M4F)
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F/port.c
    # FreeRTOS memory manager (heap_4)
    ${FREERTOS_DIR}/portable/MemMang/heap_4.c
)

# Custom middleware
target_sources(middlewares PRIVATE
    ${CMAKE_SOURCE_DIR}/Middlewares/Controller/controller.c
    ${CMAKE_SOURCE_DIR}/Middlewares/Algorithm/Include/user_lib.c
    ${CMAKE_SOURCE_DIR}/Middlewares/Filter/FirstOrderFilter/filter32.c
)
