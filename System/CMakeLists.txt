include_directories(./include)
#AUX_SOURCE_DIRECTORY(. DIR_Kernel_SRCS)
#AUX_SOURCE_DIRECTORY(./porting DIR_Kernel_SRCS)
#ADD_LIBRARY(kernel ${DIR_Kernel_SRCS})

aux_source_directory(. SYS_SRCS)

add_executable(sys.elf ${SYS_SRCS})


target_link_libraries(sys.elf -lm -lc -nostartfiles)

