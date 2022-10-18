include_directories(.)
AUX_SOURCE_DIRECTORY(. DIR_hal_SRCS)
#AUX_SOURCE_DIRECTORY(./Hardware DIR_hal_SRCS)
AUX_SOURCE_DIRECTORY(./Hardware DIR_Hardware_SRCS)
ADD_LIBRARY(hal ${DIR_hal_SRCS})
ADD_LIBRARY(hal_hardware ${DIR_Hardware_SRCS})

