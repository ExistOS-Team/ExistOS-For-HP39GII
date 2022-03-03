include_directories(./include)
include_directories(./coremarkTest)
include_directories(./lib)
include_directories(./lib/giac1.2)
include_directories(./lib/gmp)


aux_source_directory(.                          SYS_SRCS)
aux_source_directory(./coremarkTest             SYS_SRCS)



add_executable(sys.elf ${SYS_SRCS})

                    
target_link_options(sys.elf PRIVATE 
-T ${CMAKE_SOURCE_DIR}/Script/sys_ld.script 
-Wl,--gc-sections
)


target_link_libraries(sys.elf  -lm -lc -lstdc++ -nostartfiles
${CMAKE_SOURCE_DIR}/Libs/libgiac.libcpp ${CMAKE_SOURCE_DIR}/Libs/libgmp.libcpp
)


target_compile_options(sys.elf PRIVATE -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian 
-Os -pipe -fdata-sections -ffunction-sections -fcommon -fno-strict-aliasing #-fno-exceptions
)

add_custom_target(
  ExistOS.sys ALL
  COMMAND ${CMAKE_OBJCOPY} -I elf32-little -O binary -R .note -R .comment -S ${CMAKE_CURRENT_BINARY_DIR}/sys.elf ${CMAKE_CURRENT_BINARY_DIR}/ExistOS.sys 
  DEPENDS sys.elf 
  )
