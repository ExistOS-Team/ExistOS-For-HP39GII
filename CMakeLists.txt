cmake_minimum_required(VERSION 3.16)

include(ExternalProject)

set(CMAKE_TOOLCHAIN_FILE toolchain.cmake)
project(ExistOS-for-HP39gii VERSION 0.1.0)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

add_compile_options(-mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian
                    -Os -pipe -fdata-sections -ffunction-sections -fcommon)

SET(BUILD_SHARED_LIBRARIES OFF)

add_subdirectory(System)
add_subdirectory(OSLoader)

