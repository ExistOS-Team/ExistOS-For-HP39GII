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
                    
add_link_options(-T ${CMAKE_SOURCE_DIR}/script/ld.script -Wl,--gc-sections)

SET(BUILD_SHARED_LIBRARIES OFF)
SET(CMAKE_EXE_LINKER_FLAGS "-static")


include_directories(${CMAKE_SOURCE_DIR}/Config)
include_directories(${CMAKE_SOURCE_DIR}/Fonts)
include_directories(${CMAKE_SOURCE_DIR}/Scheduler/include)
include_directories(${CMAKE_SOURCE_DIR}/HAL/Hardware)
include_directories(${CMAKE_SOURCE_DIR}/HAL)
include_directories(${CMAKE_SOURCE_DIR}/HAL/Hardware/registers)
include_directories(${CMAKE_SOURCE_DIR}/Scheduler/porting)
include_directories(${CMAKE_SOURCE_DIR}/Component3rd/tinyusb)
include_directories(${CMAKE_SOURCE_DIR}/Component3rd/tinyusb/device)
include_directories(${CMAKE_SOURCE_DIR}/Component3rd/dhara)
include_directories(${CMAKE_SOURCE_DIR}/Component3rd/FatFs)

aux_source_directory(.                                                    LOADER_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/Scheduler                        LOADER_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/Scheduler/porting                LOADER_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/HAL                              LOADER_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/HAL/Hardware                     LOADER_SRCS)

aux_source_directory(${CMAKE_SOURCE_DIR}/Component3rd                     LOADER_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/Component3rd/tinyusb             LOADER_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/Component3rd/tinyusb/device      LOADER_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/Component3rd/tinyusb/common      LOADER_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/Component3rd/tinyusb/class/cdc   LOADER_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/Component3rd/tinyusb/class/msc   LOADER_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/Component3rd/dhara               LOADER_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/Component3rd/FatFs               LOADER_SRCS)

add_executable(rom.elf ${LOADER_SRCS})



add_subdirectory(System)





target_link_libraries(rom.elf -lm -lc -nostartfiles)

add_custom_command(TARGET rom.elf POST_BUILD
    COMMAND arm-none-eabi-size ${CMAKE_CURRENT_BINARY_DIR}/rom.elf 
    )
	
add_custom_target(
  rom.bin ALL
  COMMAND ${CMAKE_OBJCOPY} -I elf32-little -O binary -R .note -R .comment -S ${CMAKE_CURRENT_BINARY_DIR}/rom.elf ${CMAKE_CURRENT_BINARY_DIR}/rom.bin 
  DEPENDS rom.elf 
  )

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
add_custom_target(
  OSLoader.sb ALL
  COMMAND ${CMAKE_SOURCE_DIR}/tools/sbtools_win/elftosb -z -c ${CMAKE_SOURCE_DIR}/Script/build_fw.bd -o ${CMAKE_CURRENT_BINARY_DIR}/OSLoader.sb ${CMAKE_CURRENT_BINARY_DIR}/rom.bin
  DEPENDS rom.bin
  )
add_custom_target(
  flash 
  COMMAND ${CMAKE_SOURCE_DIR}/tools/sbtools_win/sb_loader -f ${CMAKE_CURRENT_BINARY_DIR}/OSLoader.sb 
  DEPENDS OSLoader.sb
  )
else()
if(EXISTS ${CMAKE_SOURCE_DIR}/tools/sbtools/elftosb)
add_custom_target(
  OSLoader.sb ALL
  COMMAND ${CMAKE_SOURCE_DIR}/tools/sbtools/elftosb -z -c ${CMAKE_SOURCE_DIR}/script/build_fw.bd -o ${CMAKE_CURRENT_BINARY_DIR}/OSLoader.sb ${CMAKE_CURRENT_BINARY_DIR}/rom.bin
  DEPENDS rom.bin
  )
add_custom_target(
  flash 
  COMMAND ${CMAKE_SOURCE_DIR}/tools/sbtools/sbloader ${CMAKE_CURRENT_BINARY_DIR}/OSLoader.sb -d
  DEPENDS OSLoader.sb
  )
else(EXISTS ${CMAKE_SOURCE_DIR}/tools/sbtools/elftosb)
message(FATAL_ERROR "Please build tools/sbtools first")
endif(EXISTS ${CMAKE_SOURCE_DIR}/tools/sbtools/elftosb)
endif()

