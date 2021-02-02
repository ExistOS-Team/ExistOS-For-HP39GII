include_directories(.)
AUX_SOURCE_DIRECTORY(. DIR_shell_SRCS)
ADD_LIBRARY(shell ${DIR_shell_SRCS})