

include_directories(./)
include_directories(./Core)
include_directories(./Core/include)
include_directories(./Core/porting)

include_directories(./Fs)
include_directories(./Fs/Fatfs)
include_directories(./Fs/littlefs) 

include_directories(./Config)
include_directories(./CoremarkTest)

include_directories(./KLib/lvgl)

include_directories(${CMAKE_SOURCE_DIR}/Libs/src/micropython-master/)
include_directories(${CMAKE_SOURCE_DIR}/Libs/src/micropython-master/ports/eoslib)
include_directories(${CMAKE_SOURCE_DIR}/Libs/src/micropython-master/ports/eoslib/build)
#include_directories(${CMAKE_SOURCE_DIR}/Libs/src/)
#include_directories(${CMAKE_SOURCE_DIR}/Libs/src/giac-1.4.9/)
#include_directories(${CMAKE_SOURCE_DIR}/Libs/src/giac-1.2.0/src)
#include_directories(${CMAKE_SOURCE_DIR}/Libs/src/gmp/include)
include_directories(${CMAKE_SOURCE_DIR}/Libs/src/libtommath-0.39/)
include_directories(${CMAKE_SOURCE_DIR}/Libs/src/khicas/)


aux_source_directory(.                                         SYS_SRCS)
aux_source_directory(./Core                                    SYS_SRCS)
aux_source_directory(./Core/porting                            SYS_SRCS)
aux_source_directory(./CoremarkTest                            SYS_SRCS)
aux_source_directory(./Fs                                      SYS_SRCS)
aux_source_directory(./Fs/Fatfs                                SYS_SRCS)
aux_source_directory(./Fs/littlefs                             SYS_SRCS)

#add_subdirectory(gb)
add_subdirectory(KLib)
add_subdirectory(KLib/tjpgdec)
add_subdirectory(emu48)

add_executable(sys.elf ${SYS_SRCS})

set(LINKER_SCRIPT 
${CMAKE_SOURCE_DIR}/Script/sys_ld.script
)
set(LINKER_FLAGS "SHELL:-T${LINKER_SCRIPT} -Wl,--wrap=malloc -Wl,--wrap=free")
#-Wl,--gc-sections  
target_link_options(sys.elf PRIVATE ${LINKER_FLAGS}) 

 
#target_compile_options(gb PRIVATE -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -mfloat-abi=soft -mthumb -Ofast)
target_compile_options(emu48 PRIVATE -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -mfloat-abi=soft -mthumb -Os)
target_compile_options(lvgl PRIVATE -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -mfloat-abi=soft -mthumb -Os)
target_compile_options(tjpgd PRIVATE -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -mfloat-abi=soft -mthumb-interwork -mthumb -Os)


#-mthumb
target_compile_options(sys.elf PRIVATE -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -mfloat-abi=soft -marm
-Os -pipe 
#-ffunction-sections -fno-strict-aliasing -fomit-frame-pointer
#-fcommon -fno-strict-aliasing -fomit-frame-pointer
#-fpermissive  -fdata-sections
#-fno-exceptions 
) 

#set_property(TARGET lvgl PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
#set_property(TARGET gb PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

target_link_libraries(sys.elf -lm -lc -lrdimon -lgcc -lstdc++ -nostartfiles
tjpgd
#gb 
emu48
lvgl
${CMAKE_SOURCE_DIR}/Libs/libkcasgui.libcpp
#${CMAKE_SOURCE_DIR}/Libs/libmpy.libc
${CMAKE_SOURCE_DIR}/Libs/libcas.libcpp
#${CMAKE_SOURCE_DIR}/Libs/libgiac1.4.9.libcpp
#${CMAKE_SOURCE_DIR}/Libs/libgiac1.2.0.libcpp
${CMAKE_SOURCE_DIR}/Libs/libtommath.libcpp
#${CMAKE_SOURCE_DIR}/Libs/libgmp.libcpp   
)

# build time in UTC ISO 8601
FILE (WRITE ${CMAKE_BINARY_DIR}/timestamp.cmake "STRING(TIMESTAMP TIMEZ UTC)\n")
FILE (APPEND ${CMAKE_BINARY_DIR}/timestamp.cmake "FILE(WRITE timestamp.h \"#ifndef TIMESTAMP_H\\n\")\n")
FILE (APPEND ${CMAKE_BINARY_DIR}/timestamp.cmake "FILE(APPEND timestamp.h \"#define TIMESTAMP_H\\n\\n\")\n")
FILE (APPEND ${CMAKE_BINARY_DIR}/timestamp.cmake "FILE(APPEND timestamp.h \"#define _TIMEZ_ \\\"\${TIMEZ}\\\"\\n\\n\")\n")
FILE (APPEND ${CMAKE_BINARY_DIR}/timestamp.cmake "FILE(APPEND timestamp.h \"#endif // TIMESTAMP_H\\n\")\n")

ADD_CUSTOM_TARGET (
    timestamp
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/timestamp.cmake
    ADD_DEPENDENCIES ${CMAKE_BINARY_DIR}/timestamp.cmake)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(sys.elf timestamp)

add_custom_target(
  ExistOS.sys ALL
  COMMAND ${CMAKE_OBJCOPY} -I elf32-little -O binary -R .note -R .comment -S ${CMAKE_CURRENT_BINARY_DIR}/sys.elf ${CMAKE_CURRENT_BINARY_DIR}/ExistOS.sys 
  COMMAND ${CMAKE_NM} -n ${CMAKE_CURRENT_BINARY_DIR}/sys.elf > ${CMAKE_CURRENT_BINARY_DIR}/sys_symtab.txt 
  COMMAND ${CMAKE_SOURCE_DIR}/tools/sysigner ${CMAKE_CURRENT_BINARY_DIR}/sys_symtab.txt ${CMAKE_CURRENT_BINARY_DIR}/ExistOS.sys 
  DEPENDS sys.elf 
  )

  add_custom_target(
    edb_flash_sys 
    COMMAND ${CMAKE_SOURCE_DIR}/tools/edb -r -s -f ${CMAKE_CURRENT_BINARY_DIR}/ExistOS.sys 1984
    DEPENDS ExistOS.sys
    )

add_dependencies(edb_flash_sys timestamp)
