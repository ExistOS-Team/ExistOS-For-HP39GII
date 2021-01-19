include_directories(.)
AUX_SOURCE_DIRECTORY(. DIR_dhara_SRCS)
ADD_LIBRARY(dhara ${DIR_dhara_SRCS})