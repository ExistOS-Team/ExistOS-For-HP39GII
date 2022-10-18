include_directories(.)
include_directories(./include)
AUX_SOURCE_DIRECTORY(. DIR_KERNEL_SRCS)
AUX_SOURCE_DIRECTORY(./porting DIR_KERNEL_SRCS)

ADD_LIBRARY(kernel ${DIR_KERNEL_SRCS})



