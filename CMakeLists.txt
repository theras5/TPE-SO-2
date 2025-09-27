cmake_minimum_required(VERSION 3.15)
project(Arqui_TPE C ASM)

set(CMAKE_C_STANDARD 11)

# Incluye headers
include_directories(${CMAKE_SOURCE_DIR}/Kernel/include)
include_directories(${CMAKE_SOURCE_DIR}/Userland/include)

# Archivos fuente C
set(SOURCES
        Kernel/exceptionDispatcher.c
        Kernel/idtLoader.c
        Kernel/irqDispatcher.c
        Kernel/kernel.c
        Kernel/libc.c
        Kernel/moduleLoader.c
        Kernel/sysCallDispatcher.c
        Kernel/time.c
        Kernel/time_display.c
        Kernel/pongis.c
        Kernel/include/pongis.h
)
# Archivos fuente C de Userland (todos los .c)
file(GLOB USERLAND_SOURCES Userland/*.c)


# Archivos ASM
enable_language(ASM)
set(ASM_SOURCES
        Kernel/asm/interrupts.asm
        Kernel/asm/libasm.asm
)

# Crea un ejecutable falso para análisis
add_executable(dummy ${SOURCES} ${USERLAND_SOURCES} ${ASM_SOURCES})
