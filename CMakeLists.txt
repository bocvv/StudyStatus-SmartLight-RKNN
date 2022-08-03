cmake_minimum_required(VERSION 3.10)

project(rknn_ss_demo)

option(SS_DEBUG "fst decode support" ON)
if(SS_DEBUG)
    add_definitions(-DSS_DEBUG)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w -fPIC -O3 -ffast-math -funsafe-math-optimizations -ftree-vectorize -fstack-protector-all -Wunused-local-typedefs -Wsign-compare -Wunused-variable -Wunused-value")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -w -fPIC -O3 -ffast-math -funsafe-math-optimizations -ftree-vectorize -fstack-protector-all -Wunused-local-typedefs -Wsign-compare -Wunused-variable -Wunused-value")

# rknn api
set(RKNN_API_PATH ${CMAKE_SOURCE_DIR}/thirdparty/librknn_api)
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(RKNN_API_LIB ${RKNN_API_PATH}/${CMAKE_ANDROID_ARCH_ABI}/librknn_api.so)
  set(RKNN_RT_LIB ${RKNN_API_PATH}/${CMAKE_ANDROID_ARCH_ABI}/librknnrt.so)
else()
  if (CMAKE_C_COMPILER MATCHES "aarch64")
    set(LIB_ARCH aarch64)
  else()
    set(LIB_ARCH armhf)
  endif()
  set(RKNN_API_LIB ${RKNN_API_PATH}/${LIB_ARCH}/librknn_api.so)
  set(RKNN_RT_LIB ${RKNN_API_PATH}/${LIB_ARCH}/librknnrt.so)
endif()
include_directories(${RKNN_API_PATH}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# opencv
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/thirdparty/opencv/OpenCV-android-sdk/sdk/native/jni/abi-${CMAKE_ANDROID_ARCH_ABI})
else()
  if(LIB_ARCH STREQUAL "lib")
    set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/thirdparty/opencv/opencv-linux-armhf/share/OpenCV)
  else()
    set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/thirdparty/opencv/opencv-linux-aarch64/share/OpenCV)
  endif()
endif()
find_package(OpenCV REQUIRED)

set(CMAKE_INSTALL_RPATH "lib")

add_executable(ss_demo
        src/study_status.cpp
        src/config/cls_config.cpp
        src/demo.cpp
        )

add_library(study_status SHARED
    src/study_status.cpp
    src/config/cls_config.cpp
    src/jni_lib.cpp
    )

target_link_libraries(ss_demo
	${RKNN_API_LIB}
	${OpenCV_LIBS}
)

target_link_libraries(study_status
	${RKNN_API_LIB}
	${OpenCV_LIBS}
)

# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/ss_demo_${CMAKE_SYSTEM_NAME})
install(TARGETS ss_demo DESTINATION ./)
install(DIRECTORY data DESTINATION ./)
install(TARGETS study_status DESTINATION lib)
install(PROGRAMS ${RKNN_API_LIB} DESTINATION lib)
install(PROGRAMS ${RKNN_RT_LIB} DESTINATION lib)
