cmake_minimum_required(VERSION 3.16)


project(sysigner VERSION 0.1.0)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
SET(BUILD_SHARED_LIBRARIES OFF)


include_directories(.)
include_directories(./libelf)

aux_source_directory(.              EXE_SRCS)

add_subdirectory(libelf)

add_executable(sysigner ${EXE_SRCS})
 
SET_TARGET_PROPERTIES(sysigner PROPERTIES COMPILE_FLAGS "-O0 -static" ) 


target_link_libraries(sysigner -lm -lgcc -lstdc++
libelf -static
)

