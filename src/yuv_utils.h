/*
 * yuv_utils.hpp
 * 
 * Created on: 2021/09/14 17:49:01
 *     Author: renzh
 */
#ifndef JNI_YUV_UTILS_
#define JNI_YUV_UTILS_

#include <jni.h>
#include "libyuv.h"


void nv21ToI420(jbyte *src_nv21_data, jint width, jint height, jbyte *src_i420_data);

void scaleI420(jbyte *src_i420_data, jint width, jint height, jbyte *dst_i420_data, jint dst_width,
               jint dst_height, jint mode);
void cropI420(jbyte *src_i420_data, jint width, jint height, jbyte *dst_i420_data, jint dst_width,
               jint dst_height, jint left, jint top);

//void I420ToBGR(jbyte *src_i420_data, unsigned char* dst_bgr_data, jint width, jint hight);
void I420ToRGB24(unsigned char *yuvData, unsigned char *rgb24, int width, int height);

#endif
