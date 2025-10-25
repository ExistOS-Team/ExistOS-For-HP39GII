include_directories(.)
include_directories(../drivers)
include_directories(../drivers/display)
include_directories(../drivers/input)
include_directories(../drivers/timer)
include_directories(./Hardware/registers)

AUX_SOURCE_DIRECTORY(. DIR_hal_SRCS)
AUX_SOURCE_DIRECTORY(./Hardware DIR_hal_SRCS)  # Old Hardware directory

# New drivers directory structure
AUX_SOURCE_DIRECTORY(../drivers DIR_drivers_SRCS)
AUX_SOURCE_DIRECTORY(../drivers/display DIR_display_SRCS)
AUX_SOURCE_DIRECTORY(../drivers/input DIR_input_SRCS)
AUX_SOURCE_DIRECTORY(../drivers/timer DIR_timer_SRCS)

ADD_LIBRARY(hal ${DIR_hal_SRCS})
ADD_LIBRARY(drivers ${DIR_drivers_SRCS})
ADD_LIBRARY(display ${DIR_display_SRCS})
ADD_LIBRARY(input ${DIR_input_SRCS})
ADD_LIBRARY(timer ${DIR_timer_SRCS})

