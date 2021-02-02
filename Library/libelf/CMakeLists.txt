include_directories(./inc)
AUX_SOURCE_DIRECTORY(./src DIR_libelf_SRCS)
ADD_LIBRARY(libelf ${DIR_libelf_SRCS})