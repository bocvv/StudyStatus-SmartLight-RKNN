LOCAL_PATH := $(call my-dir)

OpenCV_BASE = $(LOCAL_PATH)/thirdparty/opencv/OpenCV-android-sdk
LIBYUV_BASE = $(LOCAL_PATH)/thirdparty/libyuv/android
SS_BASE = ./src
RKNN_BASE = $(LOCAL_PATH)/thirdparty/librknn_api
MEDIA_ENCODER_BASE = $(LOCAL_PATH)/thirdparty/media_enc
MEDIA_ENCODER_INCLUDE_DIR = $(MEDIA_ENCODER_BASE)/include
RKNN_INCLUDE_DIR = $(RKNN_BASE)/include

include $(CLEAR_VARS)

LOCAL_MODULE := media_encoder
LOCAL_SRC_FILES := $(MEDIA_ENCODER_BASE)/lib/libmedia_encoder.so
include $(PREBUILT_SHARED_LIBRARY)
$(warning "MEDIA_ENCODER include dir $(MEDIA_ENCODER_INCLUDE_DIR)")

include $(CLEAR_VARS)

LOCAL_MODULE := mpp
LOCAL_SRC_FILES := $(MEDIA_ENCODER_BASE)/lib/libmpp.so
include $(PREBUILT_SHARED_LIBRARY)
$(warning "MPP include dir $(MEDIA_ENCODER_INCLUDE_DIR)")

include $(CLEAR_VARS)

LOCAL_MODULE := vpu
LOCAL_SRC_FILES := $(MEDIA_ENCODER_BASE)/lib/libvpu.so
include $(PREBUILT_SHARED_LIBRARY)
$(warning "VPU include dir $(MEDIA_ENCODER_INCLUDE_DIR)")

include $(CLEAR_VARS)

LOCAL_MODULE := AladdinSDKLog
LOCAL_SRC_FILES := $(MEDIA_ENCODER_BASE)/lib/libAladdinSDKLog.so
include $(PREBUILT_SHARED_LIBRARY)
$(warning "AladdinSDKLog include dir $(MEDIA_ENCODER_INCLUDE_DIR)")

include $(CLEAR_VARS)

LOCAL_MODULE := rknnrt
LOCAL_SRC_FILES := $(RKNN_BASE)/$(TARGET_ARCH_ABI)/librknnrt.so
include $(PREBUILT_SHARED_LIBRARY)
$(warning "RKNN include dir $(RKNN_INCLUDE_DIR)")

include $(CLEAR_VARS)

LOCAL_MODULE := yuv
LOCAL_SRC_FILES := $(LIBYUV_BASE)/lib/$(TARGET_ARCH_ABI)/libyuv.a
include $(PREBUILT_STATIC_LIBRARY)
LIBYUV_INCLUDE_DIR = $(LIBYUV_BASE)/include
$(warning "YUV include dir $(LIBYUV_INCLUDE_DIR)")

include $(CLEAR_VARS)

OPENCV_INSTALL_MODULES := on
OPENCV_LIB_TYPE := SHARED
include $(OpenCV_BASE)/sdk/native/jni/OpenCV.mk
OPENCV_INCLUDE_DIR = $(OpenCV_BASE)/sdk/native/jni/include
$(warning "opencv include dir $(OPENCV_INCLUDE_DIR)")

LOCAL_C_INCLUDES += $(RKNN_INCLUDE_DIR)
LOCAL_C_INCLUDES += $(OPENCV_INCLUDE_DIR)
LOCAL_C_INCLUDES += $(SS_BASE)
LOCAL_C_INCLUDES += $(LIBYUV_INCLUDE_DIR)
LOCAL_C_INCLUDES += $(MEDIA_ENCODER_INCLUDE_DIR)

LOCAL_SRC_FILES := $(SS_BASE)/base_util.cpp \
				   $(SS_BASE)/study_status.cpp \
				   $(SS_BASE)/config/cls_config.cpp \
				   $(SS_BASE)/yuv_utils.cpp

#LOCAL_SRC_FILES += $(SS_BASE)/demo.cpp
LOCAL_SRC_FILES += $(SS_BASE)/jni_lib.cpp

LOCAL_LDLIBS += -llog -ldl -lm 
LOCAL_CFLAGS   := -O2 -fvisibility=hidden -fomit-frame-pointer -fstrict-aliasing -ffunction-sections -fdata-sections -ffast-math -ftree-vectorize -fPIC -Ofast -ffast-math -w -std=c++14 -fuse-ld=lld 
LOCAL_CPPFLAGS := -O2 -fvisibility=hidden -fvisibility-inlines-hidden -fomit-frame-pointer -fstrict-aliasing -ffunction-sections -fdata-sections -ffast-math -fPIC -Ofast -ffast-math -std=c++14 -fuse-ld=lld
LOCAL_LDFLAGS  += -Wl,--gc-sections

ifdef IS_FLAG
	LOCAL_CFLAGS  += -DSS_DEBUG
	LOCAL_CPPFLAGS  += -DSS_DEBUG
endif
LOCAL_CFLAGS  += -DUSE_MODEL_ENCRYPT
LOCAL_CPPFLAGS  += -DUSE_MODEL_ENCRYPT

#LOCAL_SHARED_LIBRARIES := rknnrt opencv_java3 media_encoder AladdinSDKLog mpp vpu
LOCAL_SHARED_LIBRARIES := rknnrt opencv_java3
LOCAL_STATIC_LIBRARIES := yuv

#LOCAL_MODULE     := ndk_ss_demo
LOCAL_MODULE     := study_status

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_EXECUTABLE)
