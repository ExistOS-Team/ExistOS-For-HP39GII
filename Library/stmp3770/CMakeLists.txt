include_directories(./inc)
include_directories(./inc/registers)
AUX_SOURCE_DIRECTORY(./src DIR_stmp3770_SRCS)
ADD_LIBRARY(stmp3770 ${DIR_stmp3770_SRCS})