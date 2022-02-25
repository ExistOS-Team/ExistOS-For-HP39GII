include_directories(./include)


aux_source_directory(. SYS_SRCS)

add_executable(sys.elf ${SYS_SRCS})

                    
target_link_options(sys.elf PRIVATE 
-T ${CMAKE_SOURCE_DIR}/Script/sys_ld.script 
-Wl,--gc-sections
)
target_link_libraries(sys.elf -lm -lc -nostartfiles)

add_custom_target(
  ExistOS.sys ALL
  COMMAND ${CMAKE_OBJCOPY} -I elf32-little -O binary -R .note -R .comment -S ${CMAKE_CURRENT_BINARY_DIR}/sys.elf ${CMAKE_CURRENT_BINARY_DIR}/ExistOS.sys 
  DEPENDS sys.elf 
  )


