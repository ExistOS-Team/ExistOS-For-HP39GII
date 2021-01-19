include_directories(./include)
AUX_SOURCE_DIRECTORY(. DIR_Kernel_SRCS)
AUX_SOURCE_DIRECTORY(./porting DIR_Kernel_SRCS)
ADD_LIBRARY(kernel ${DIR_Kernel_SRCS})
