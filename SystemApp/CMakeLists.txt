include_directories(./inc)
AUX_SOURCE_DIRECTORY(./src DIR_systemapp_SRCS)
ADD_LIBRARY(systemapp ${DIR_systemapp_SRCS})