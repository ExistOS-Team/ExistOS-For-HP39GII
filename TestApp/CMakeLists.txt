include_directories(.)
AUX_SOURCE_DIRECTORY(. DIR_testapp_SRCS)
ADD_LIBRARY(testapp ${DIR_testapp_SRCS})