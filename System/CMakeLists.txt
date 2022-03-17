include_directories(./)
include_directories(./Config)
include_directories(./coremarkTest)
include_directories(${CMAKE_SOURCE_DIR}/Libs/src/)
#include_directories(${CMAKE_SOURCE_DIR}/Libs/src/giac-1.4.9/)
include_directories(${CMAKE_SOURCE_DIR}/Libs/src/giac-1.2.0/src)
include_directories(${CMAKE_SOURCE_DIR}/Libs/src/gmp/include)



aux_source_directory(.                                         SYS_SRCS)
aux_source_directory(./coremarkTest                            SYS_SRCS)


add_executable(sys.elf ${SYS_SRCS})

                    
#target_link_options(sys.elf PRIVATE 
#-T ${CMAKE_SOURCE_DIR}/Script/sys_ld.script 
#-Wl,--gc-sections
#)

set(LINKER_SCRIPT 
${CMAKE_SOURCE_DIR}/Script/sys_ld.script
)
set(LINKER_FLAGS "SHELL:-T${LINKER_SCRIPT} -Wl,--gc-sections")
target_link_options(sys.elf PRIVATE ${LINKER_FLAGS})

target_link_libraries(sys.elf  -lm -lc -lstdc++ -nostartfiles
#${CMAKE_SOURCE_DIR}/Libs/libgiac1.4.9.libcpp
${CMAKE_SOURCE_DIR}/Libs/libgiac1.2.0.libcpp
#${CMAKE_SOURCE_DIR}/Libs/libtommath.libcpp
${CMAKE_SOURCE_DIR}/Libs/libgmp.libcpp
)


target_compile_options(sys.elf PRIVATE -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian 
-O3 -pipe -fdata-sections -ffunction-sections -fcommon -fno-strict-aliasing 
#-fpermissive
#-fno-exceptions
)

add_custom_target(
  ExistOS.sys ALL
  COMMAND ${CMAKE_OBJCOPY} -I elf32-little -O binary -R .note -R .comment -S ${CMAKE_CURRENT_BINARY_DIR}/sys.elf ${CMAKE_CURRENT_BINARY_DIR}/ExistOS.sys 
  DEPENDS sys.elf 
  )
