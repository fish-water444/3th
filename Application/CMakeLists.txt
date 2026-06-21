# Application library: task modules
add_library(app STATIC)

target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/Application/ChassisTask/chassis_task.c
)
