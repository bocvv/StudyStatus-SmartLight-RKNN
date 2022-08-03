cmake_minimum_required(VERSION 2.8.10)
project(SS_TOOL)

set (CMAKE_CXX_STANDARD 11)
if (CMAKE_HOST_WIN32)
    set(WINDOWS 1)
elseif (CMAKE_HOST_APPLE)
    set(MACOS 1)
elseif (CMAKE_HOST_UNIX)
    set(LINUX 1)
endif ()


message("macos env config")
set(CMAKE_MACOSX_RPATH 1)

# 用于找到opencv编译好的各种东西
include_directories(thirdparty/macos/opencv-3.4.10/include)
include_directories(thirdparty/macos/opencv-3.4.10/include/opencv)
#include_directories(thirdparty/macos/opencv-3.4.10/include/opencv2)
link_directories(thirdparty/macos/opencv-3.4.10/lib)

set(opencv_LIBS
    opencv_world.3.4
)

include_directories(./)
add_executable(ss_tool decode_base64_to_image.cpp base64.cpp)
target_link_libraries(ss_tool ${opencv_LIBS})

