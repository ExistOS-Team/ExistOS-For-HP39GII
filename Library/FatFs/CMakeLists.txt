include_directories(.)
AUX_SOURCE_DIRECTORY(. DIR_FatFs_SRCS)
ADD_LIBRARY(FatFs ${DIR_FatFs_SRCS})