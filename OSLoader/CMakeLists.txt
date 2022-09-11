cmake_minimum_required(VERSION 3.16)

project(ExistOS-OSLoader VERSION 0.1.0.1)


add_link_options(-T ${CMAKE_SOURCE_DIR}/Script/ldr_ld.script -Wl,--gc-sections)

SET(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_BUILD_TYPE "MinSizeRel") 

include_directories(./Config)
include_directories(./Fonts)
include_directories(./Scheduler/include)
include_directories(./Include)
include_directories(./HAL/Hardware)
include_directories(./HAL)
include_directories(./HAL/Hardware/registers)
include_directories(./VmMgr)
include_directories(./LowLevelAPI)
include_directories(./Scheduler/porting)
include_directories(./Component3rd/tinyusb)
include_directories(./Component3rd/tinyusb/device)
include_directories(./Component3rd/tinyusb/class/hid)
include_directories(./Component3rd/tinyusb/class/net)
include_directories(./Component3rd/tinyusb/class/vendor)
include_directories(./Component3rd/dhara)
#include_directories(./Component3rd/FatFs)

aux_source_directory(.                                  LOADER_SRCS)
aux_source_directory(./Scheduler                        LOADER_SRCS)
aux_source_directory(./VmMgr                            LOADER_SRCS)
aux_source_directory(./LowLevelAPI                      LOADER_SRCS)
aux_source_directory(./Scheduler/porting                LOADER_SRCS)
aux_source_directory(./HAL                              LOADER_SRCS)
aux_source_directory(./HAL/Hardware                     LOADER_SRCS)

aux_source_directory(./Component3rd                     LOADER_SRCS)
aux_source_directory(./Component3rd/tinyusb             LOADER_SRCS)
aux_source_directory(./Component3rd/tinyusb/device      LOADER_SRCS)
aux_source_directory(./Component3rd/tinyusb/common      LOADER_SRCS)
aux_source_directory(./Component3rd/tinyusb/class/cdc   LOADER_SRCS)
aux_source_directory(./Component3rd/tinyusb/class/msc   LOADER_SRCS)
aux_source_directory(./Component3rd/tinyusb/class/hid   LOADER_SRCS)
aux_source_directory(./Component3rd/tinyusb/class/net   LOADER_SRCS)
aux_source_directory(./Component3rd/tinyusb/class/vendor   LOADER_SRCS)
aux_source_directory(./Component3rd/dhara               LOADER_SRCS)
#aux_source_directory(./Component3rd/FatFs               LOADER_SRCS)


add_executable(rom.elf ${LOADER_SRCS})

target_link_libraries(rom.elf PRIVATE -lc_nano -nostartfiles)

target_compile_options(rom.elf PRIVATE -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -marm -Wall
#-O2
-Os -DNDEBUG
-pipe 
-fdata-sections 
-ffunction-sections
-fdelete-null-pointer-checks 
-fcommon 
-finline-functions 
-finline-small-functions

-fauto-inc-dec 
-fcombine-stack-adjustments 
-fcompare-elim 
-fcprop-registers 
-fdce 
-fdefer-pop 
-fdse 
-fforward-propagate 
-fguess-branch-probability 
-fif-conversion 
-fif-conversion2 
-finline-functions-called-once 
-fmerge-constants 
-fmove-loop-invariants 
-fomit-frame-pointer 
-freorder-blocks 
-fshrink-wrap 
-fshrink-wrap-separate 
-fsplit-wide-types 
-fssa-backprop 
-fssa-phiopt 
-ftree-bit-ccp 
-ftree-ccp 
-ftree-ch 
-ftree-coalesce-vars 
-ftree-copy-prop 
-ftree-dce 
-ftree-dominator-opts 
-ftree-dse 
-ftree-forwprop 
-ftree-fre 
-ftree-phiprop 
-ftree-pta 
-ftree-scev-cprop 
-ftree-sink 
-ftree-slsr 
-ftree-sra 
-ftree-ter 
-funit-at-a-time

-fcaller-saves 
-fcode-hoisting 
-fcrossjumping 
-fcse-follow-jumps  -fcse-skip-blocks 
-fdelete-null-pointer-checks 
-fdevirtualize  -fdevirtualize-speculatively 
-fexpensive-optimizations 
-ffinite-loops 
#-finline-functions 
#-finline-small-functions 
#-findirect-inlining 
-foptimize-sibling-calls 
-foptimize-strlen 
-fpartial-inlining 
-fpeephole2 
-frerun-cse-after-loop  
-fstore-merging 
-fstrict-aliasing 
-fthread-jumps 
-ftree-builtin-call-dce 
-ftree-loop-vectorize 
-ftree-pre 
-ftree-slp-vectorize 
-ftree-switch-conversion  -ftree-tail-merge 
-ftree-vrp 

)
 

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
  sb_flash 
  COMMAND ${CMAKE_SOURCE_DIR}/tools/sbtools_win/sb_loader -f ${CMAKE_CURRENT_BINARY_DIR}/OSLoader.sb 
  DEPENDS OSLoader.sb
  )

add_custom_target(
    edb_flash_loader 
    COMMAND ${CMAKE_SOURCE_DIR}/tools/edb -r -s -f ${CMAKE_CURRENT_BINARY_DIR}/OSLoader.sb 1408 b
    DEPENDS OSLoader.sb
    )

add_custom_target(
      mscmode 
      COMMAND ${CMAKE_SOURCE_DIR}/tools/edb -m
      )

add_custom_target(
    reboot 
    COMMAND ${CMAKE_SOURCE_DIR}/tools/edb -r
    )
else()
if(EXISTS ${CMAKE_SOURCE_DIR}/tools/sbtools/  )
add_custom_target(
  OSLoader.sb ALL
  COMMAND ${CMAKE_SOURCE_DIR}/tools/sbtools/elftosb -z -c ${CMAKE_SOURCE_DIR}/Script/build_fw.bd -o ${CMAKE_CURRENT_BINARY_DIR}/OSLoader.sb ${CMAKE_CURRENT_BINARY_DIR}/rom.bin
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

