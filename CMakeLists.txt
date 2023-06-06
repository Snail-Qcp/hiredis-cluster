cmake_minimum_required(VERSION 3.1)

project(hiredis)

# 查找并添加多个源文件
file(GLOB SRC_FILES
     adlist.c
     async.c
     command.c
     crc16.c
     dict.c
     hiarray.c
     hircluster.c
     hiredis.c
     hiutil.c
     net.c
     read.c
     sds.c
     )

add_library(${PROJECT_NAME} ${SRC_FILES})

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../libredis)

message("lib output path: ${LIBRARY_OUTPUT_PATH}")

# 设置多个编译器选项
if (WIN32)
    add_definitions(
        -DWIN32
        -D_DEBUG
        -D_CONSOLE
        -D__WINDOWS__
        -D_CRT_SECURE_NO_WARNINGS
        -D_SCL_SECURE_NO_WARNINGS
    )

    set(CMAKE_C_FLAGS_DEBUG   "/WX- /EHsc /nologo /RTC1 /Gd /MTd /MP /GS /W3 /Gy- /Zc:wchar_t /Gm- /Od /Zi /fp:precise /Zc:forScope")
    set(CMAKE_CXX_FLAGS_DEBUG "/WX- /EHsc /nologo /RTC1 /Gd /MTd /MP /GS /W3 /Gy- /Zc:wchar_t /Gm- /Od /Zi /fp:precise /Zc:forScope")
elseif(UNIX)
    add_definitions(-D__LINUX__ -DNDEBUG -D_CONSOLE)
    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(${PROJECT_NAME} PUBLIC -g -Wno-deprecated -Wwrite-strings -Wint-to-pointer-cast)
    elseif (CMAKE_BUILD_TYPE MATCHES "Release")
        target_compile_options(${PROJECT_NAME} PUBLIC -g -O2 -Wno-deprecated -Wwrite-strings -Wint-to-pointer-cast)
    endif()
endif()

add_subdirectory(test)
