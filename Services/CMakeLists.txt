include_directories(./inc)
AUX_SOURCE_DIRECTORY(./src DIR_services_SRCS)
ADD_LIBRARY(services ${DIR_services_SRCS})