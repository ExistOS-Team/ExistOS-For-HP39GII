cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_TOOLCHAIN_FILE ../toolchain.cmake)

project(TestELF)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

add_compile_options(-Wall -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -msoft-float -Os -pipe -fomit-frame-pointer -fdata-sections -ffunction-sections -fcommon)
add_link_options(-T ${CMAKE_SOURCE_DIR}/ld.script -lm)

SET(BUILD_SHARED_LIBRARIES OFF) 
SET(CMAKE_EXE_LINKER_FLAGS "-static")

include_directories(.)
aux_source_directory(. DIR_SRCS)
add_executable(test.elf ${DIR_SRCS})
add_custom_command(TARGET test.elf POST_BUILD
    COMMAND arm-none-eabi-size ${CMAKE_CURRENT_BINARY_DIR}/test.elf
    )