include_directories(.)
include_directories(./include)
AUX_SOURCE_DIRECTORY(. DIR_coreJSON_SRCS)
ADD_LIBRARY(coreJSON ${DIR_coreJSON_SRCS})