#这个工程根目录中的cmake文件相当于整个工程的cmake文件的入口
#所有cmake文件都会从这里进入，然后去具体的某个子目录中去编译其他子目录中的cmake文件
cmake_minimum_required(VERSION 3.0)
project(main)

#配置编译选项，表示编译后的可执行文件可以被gdb调试
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

#设置可执行文件最终存储的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#指定搜索的子目录,cmake会自动去找src目录中的cmake文件去编译
add_subdirectory(src)

#配置头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
