cmake_minimum_required(VERSION 3.1)

project(hiredis-test)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)

# 查找并添加多个源文件
file(GLOB SRC_FILES
     test_cluster.cpp
     )

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_directories(${PROJECT_NAME} PUBLIC ../)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} hiredis ws2_32)
else ()
    target_link_libraries(${PROJECT_NAME} hiredis)
endif ()
