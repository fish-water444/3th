# ARM Cortex-M4 bare-metal toolchain file (arm-none-eabi-gcc)

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Cross-compiler prefix
set(TOOLCHAIN_PREFIX arm-none-eabi-)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# C compiler
set(CMAKE_C_COMPILER    ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER  ${TOOLCHAIN_PREFIX}gcc)

# C++ is not used in this project
set(CMAKE_CXX_COMPILER  ${TOOLCHAIN_PREFIX}g++)

set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump CACHE INTERNAL "objdump tool")
set(CMAKE_SIZE    ${TOOLCHAIN_PREFIX}size    CACHE INTERNAL "size tool")

# Common flags
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS_INIT   "${CPU_FLAGS}" CACHE STRING "C flags init")
set(CMAKE_ASM_FLAGS_INIT "${CPU_FLAGS}" CACHE STRING "ASM flags init")

set(CMAKE_EXE_LINKER_FLAGS_INIT "${CPU_FLAGS}" CACHE STRING "Linker flags init")
