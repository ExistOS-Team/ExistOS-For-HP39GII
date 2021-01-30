cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_TOOLCHAIN_FILE toolchain.cmake)
project(ExistOS-for-HP39gii VERSION 0.1.0)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

add_compile_options(-mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -Os -pipe -ffunction-sections -fcommon)
add_link_options(-T ${CMAKE_SOURCE_DIR}/script/ld.script -Wl,--gc-sections -Wl,--wrap=malloc  -Wl,--wrap=free -Wl,--wrap=_malloc_r)

SET(BUILD_SHARED_LIBRARIES OFF)
SET(CMAKE_EXE_LINKER_FLAGS "-static")


include_directories(${CMAKE_SOURCE_DIR}/Config)
include_directories(${CMAKE_SOURCE_DIR}/Kernel/include)
include_directories(${CMAKE_SOURCE_DIR}/Kernel/porting)
include_directories(${CMAKE_SOURCE_DIR}/Library/coreJSON)
include_directories(${CMAKE_SOURCE_DIR}/Library/dhara)
include_directories(${CMAKE_SOURCE_DIR}/Library/FatFs)
include_directories(${CMAKE_SOURCE_DIR}/Library/FreeRTOS-Plus-CLI)
include_directories(${CMAKE_SOURCE_DIR}/Library/libelf/inc)
include_directories(${CMAKE_SOURCE_DIR}/Library/shell)
include_directories(${CMAKE_SOURCE_DIR}/Library/stmp3770/inc)
include_directories(${CMAKE_SOURCE_DIR}/Library/stmp3770/inc/registers)
include_directories(${CMAKE_SOURCE_DIR}/Library/TinyUSB)
include_directories(${CMAKE_SOURCE_DIR}/Library/TinyUSB/class/msc)
include_directories(${CMAKE_SOURCE_DIR}/Library/TinyUSB/class/cdc)
include_directories(${CMAKE_SOURCE_DIR}/Library/TinyUSB/common)
include_directories(${CMAKE_SOURCE_DIR}/Library/TinyUSB/device)
include_directories(${CMAKE_SOURCE_DIR}/Services/inc)
include_directories(${CMAKE_SOURCE_DIR}/SystemApp/inc)
include_directories(${CMAKE_SOURCE_DIR}/TestApp)
include_directories(${CMAKE_SOURCE_DIR}/)

add_subdirectory(Config)
add_subdirectory(Kernel)
add_subdirectory(Library)
add_subdirectory(Services)
add_subdirectory(SystemApp)
add_subdirectory(TestApp)

aux_source_directory(. DIR_SRCS)


add_executable(rom.elf ${DIR_SRCS})
target_link_libraries(rom.elf config kernel libraries services testapp systemapp -lm)


add_custom_command(TARGET rom.elf POST_BUILD
    COMMAND arm-none-eabi-size ${CMAKE_CURRENT_BINARY_DIR}/rom.elf 
    )
  
add_custom_target(
  rom.bin ALL
  COMMAND ${CMAKE_OBJCOPY} -I elf32-little -O binary -R .note -R .comment -S --pad-to=0x44000 ${CMAKE_CURRENT_BINARY_DIR}/rom.elf ${CMAKE_CURRENT_BINARY_DIR}/rom.bin 
  DEPENDS rom.elf 
  )

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
add_custom_target(
  firmware.sb ALL
  COMMAND ${CMAKE_SOURCE_DIR}/tools/sbtools_win/elftosb -z -c ${CMAKE_SOURCE_DIR}/script/build_fw.bd -o ${CMAKE_CURRENT_BINARY_DIR}/firmware.sb ${CMAKE_CURRENT_BINARY_DIR}/rom.bin
  DEPENDS rom.bin
  )
add_custom_target(
  flash 
  COMMAND ${CMAKE_SOURCE_DIR}/tools/sbtools_win/sb_loader -f ${CMAKE_CURRENT_BINARY_DIR}/firmware.sb 
  DEPENDS firmware.sb
  )
else()
if(EXISTS ${CMAKE_SOURCE_DIR}/tools/sbtools/elftosb)
add_custom_target(
  firmware.sb ALL
  COMMAND ${CMAKE_SOURCE_DIR}/tools/sbtools/elftosb -z -c ${CMAKE_SOURCE_DIR}/script/build_fw.bd -o ${CMAKE_CURRENT_BINARY_DIR}/firmware.sb ${CMAKE_CURRENT_BINARY_DIR}/rom.bin
  DEPENDS rom.bin
  )
add_custom_target(
  flash 
  COMMAND ${CMAKE_SOURCE_DIR}/tools/sbtools/sbloader ${CMAKE_CURRENT_BINARY_DIR}/firmware.sb -d
  DEPENDS firmware.sb
  )
else(EXISTS ${CMAKE_SOURCE_DIR}/tools/sbtools/elftosb)
message(FATAL_ERROR "Please build tools/sbtools first")
endif(EXISTS ${CMAKE_SOURCE_DIR}/tools/sbtools/elftosb)
endif()