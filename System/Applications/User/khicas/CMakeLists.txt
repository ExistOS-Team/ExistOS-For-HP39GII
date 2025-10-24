# KhiCAS application CMakeLists.txt

# 添加源文件
aux_source_directory(. KHICAS_SRCS)

# 创建静态库
add_library(khicas_lib STATIC ${KHICAS_SRCS})

# 设置库属性
target_include_directories(khicas_lib PUBLIC 
    .
    ${CMAKE_SOURCE_DIR}/Libs/src/khicas/
)

# 编译选项
target_compile_options(khicas_lib PRIVATE 
    -mtune=arm926ej-s 
    -mcpu=arm926ej-s 
    -mlittle-endian 
    -mfloat-abi=soft 
    -marm
    -Os 
    -pipe 
    -DHP39
)