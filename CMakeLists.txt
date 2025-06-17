# 指定最低版本
cmake_minimum_required(VERSION 3.5)

# 项目名称
project (NetDisk)

# 由于config_loader.cpp中的std::call_once有bug ，所以要为 GCC/Clang 添加 -pthread 解决
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()

# 引入头文件路径
include_directories(./include)
include_directories(./include/third_lib/spdlog)

# 获取某目录下的所有源文件
aux_source_directory(./src MAIN)
aux_source_directory(./src/utils UTILS)
aux_source_directory(./src/server SERVER)
aux_source_directory(./src/threadpool THREADPOOL)



# 生成 Makefile
add_executable(netdisk.out ${MAIN} ${UTILS} ${SERVER} ${THREADPOOL})