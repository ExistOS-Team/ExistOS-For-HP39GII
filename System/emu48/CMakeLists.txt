
include_directories(.)
include_directories(./core)

AUX_SOURCE_DIRECTORY(. DIR_EMU48_SRCS)
AUX_SOURCE_DIRECTORY(./core/ DIR_EMU48_SRCS)

ADD_LIBRARY(emu48 ${DIR_EMU48_SRCS})



