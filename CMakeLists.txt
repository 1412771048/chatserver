cmake_minimum_required(VERSION 3.22.1)
project(chat) #顶级cmake要写
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin) #程序生成位置
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib) #静态库生成位置
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib) #动态库生成位置
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g") #调试
include_directories(${CMAKE_SOURCE_DIR}/third ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/server) #头文件路径
# link_directories(${CMAKE_SOURCE_DIR}/lib) #第3方库路径
#以上几句只需要在顶级cmake设置即可, 次级不需要指定


# file(GLOB_RECURSE SRC "${CMAKE_SOURCE_DIR}/src/*.cpp") #收集src下所有cpp,次级一般用aux_source_directory(. SRC) 
# add_executable(shared_bike ${SRC}) #生成可执行程序,add_library()是生成库，默认静态
# target_link_libraries(shared_bike common iniparser log4cpp) #指定库名

add_subdirectory(src/server) #进入子目录
add_subdirectory(src/client)
