# comment this and add -D CMAKE_TOOLCHAIN_FILE=[CMAKE_TOOLCHAIN_FILE path] in CLion configuration
set(CMAKE_TOOLCHAIN_FILE ~/Dev/github/energia-cmake/cmake/EnergiaToolchain.cmake)

cmake_minimum_required(VERSION 3.1)
project(Timers)

set(CMAKE_VERBOSE_MAKEFILE on)

file(GLOB INC_FILES
    *.h)

file(GLOB SRC_FILES
    *.cpp)

generate_energia_library(dolink
    HDRS ${INC_FILES}
    SRCS ${SRC_FILES}
    BOARD lpcc3200)

add_subdirectory(examples/Blink)