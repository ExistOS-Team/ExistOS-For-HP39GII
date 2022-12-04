include_directories(.)
#AUX_SOURCE_DIRECTORY(. DIR_vmmgr_SRCS)
ADD_LIBRARY(vmmgr ./mapList.c ./vmMgr.c)
ADD_LIBRARY(quicklz ./quicklz.c)
ADD_LIBRARY(tlsf ./tlsf/tlsf.c)
ADD_LIBRARY(minilzo ./minilzo.c)
ADD_LIBRARY(vmmgr_mmu ./mmu.c)



