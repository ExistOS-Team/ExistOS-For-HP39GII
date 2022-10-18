
include_directories(./tinyusb)
AUX_SOURCE_DIRECTORY(./tinyusb DIR_TUSB_SRCS)
AUX_SOURCE_DIRECTORY(./tinyusb/common DIR_TUSB_SRCS)
AUX_SOURCE_DIRECTORY(./tinyusb/device DIR_TUSB_SRCS)
AUX_SOURCE_DIRECTORY(./tinyusb/class/cdc DIR_TUSB_SRCS)
AUX_SOURCE_DIRECTORY(./tinyusb/class/msc DIR_TUSB_SRCS)


include_directories(./dhara)
AUX_SOURCE_DIRECTORY(./dhara DIR_DHARA_SRCS)




ADD_LIBRARY(tusb ${DIR_TUSB_SRCS}) 
ADD_LIBRARY(dhara ${DIR_DHARA_SRCS}) 



