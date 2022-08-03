/*
 * yuv_utils.cpp
 * 
 * Created on: 2021/09/14 17:49:32
 *     Author: renzh
 */

#include "yuv_utils.h"

void nv21ToI420(jbyte *src_nv21_data, jint width, jint height, jbyte *src_i420_data) {
    jint src_y_size = width * height;
    jint src_u_size = (width >> 1) * (height >> 1);

    jbyte *src_nv21_y_data = src_nv21_data;
    jbyte *src_nv21_vu_data = src_nv21_data + src_y_size;

    jbyte *src_i420_y_data = src_i420_data;
    jbyte *src_i420_u_data = src_i420_data + src_y_size;
    jbyte *src_i420_v_data = src_i420_data + src_y_size + src_u_size;

    libyuv::NV21ToI420((const unsigned char *) src_nv21_y_data, width,
                       (const unsigned char *) src_nv21_vu_data, width,
                       (unsigned char *) src_i420_y_data, width,
                       (unsigned char *) src_i420_u_data, width >> 1,
                       (unsigned char *) src_i420_v_data, width >> 1,
                       width, height);
    return;
}

void scaleI420(jbyte *src_i420_data, jint width, jint height, jbyte *dst_i420_data, jint dst_width,
               jint dst_height, jint mode) {

    jint src_i420_y_size = width * height;
    jint src_i420_u_size = (width >> 1) * (height >> 1);
    jbyte *src_i420_y_data = src_i420_data;
    jbyte *src_i420_u_data = src_i420_data + src_i420_y_size;
    jbyte *src_i420_v_data = src_i420_data + src_i420_y_size + src_i420_u_size;

    jint dst_i420_y_size = dst_width * dst_height;
    jint dst_i420_u_size = (dst_width >> 1) * (dst_height >> 1);
    jbyte *dst_i420_y_data = dst_i420_data;
    jbyte *dst_i420_u_data = dst_i420_data + dst_i420_y_size;
    jbyte *dst_i420_v_data = dst_i420_data + dst_i420_y_size + dst_i420_u_size;

    libyuv::I420Scale((const unsigned char *) src_i420_y_data, width,
                      (const unsigned char *) src_i420_u_data, width >> 1,
                      (const unsigned char *) src_i420_v_data, width >> 1,
                      width, height,
                      (unsigned char *) dst_i420_y_data, dst_width,
                      (unsigned char *) dst_i420_u_data, dst_width >> 1,
                      (unsigned char *) dst_i420_v_data, dst_width >> 1,
                      dst_width, dst_height,
                      (libyuv::FilterMode) mode);
    return;
}

void cropI420(jbyte *src_i420_data, jint width, jint height, jbyte *dst_i420_data, jint dst_width, jint dst_height, jint left, jint top) {

    //裁剪的区域大小不对
    if (left + dst_width > width || top + dst_height > height) {
        return;
    }

    //left和top必须为偶数，否则显示会有问题
    if (left % 2 != 0 || top % 2 != 0) {
        return;
    }

    jint src_length = width*height*3/2;
    jint dst_i420_y_size = dst_width * dst_height;
    jint dst_i420_u_size = (dst_width >> 1) * (dst_height >> 1);

    jbyte *dst_i420_y_data = dst_i420_data;
    jbyte *dst_i420_u_data = dst_i420_data + dst_i420_y_size;
    jbyte *dst_i420_v_data = dst_i420_data + dst_i420_y_size + dst_i420_u_size;

    libyuv::ConvertToI420((const unsigned char *) src_i420_data, src_length,
                      (unsigned char *) dst_i420_y_data, dst_width,
                      (unsigned char *) dst_i420_u_data, dst_width >> 1,
                      (unsigned char *) dst_i420_v_data, dst_width >> 1,
                      left, top,
                      width, height,
                      dst_width, dst_height,
                      libyuv::kRotate0, libyuv::FOURCC_I420);
    
    return;
}

void I420ToRGB24(unsigned char *yuvData, unsigned char *rgb24, int width, int height) {

    unsigned char *ybase = yuvData;
    unsigned char *ubase = &yuvData[width * height];
    unsigned char *vbase = &yuvData[width * height * 5 / 4];
    //YUV420P转RGB24
    libyuv::I420ToRGB24(ybase, width, ubase, width / 2, vbase, width / 2,
                        rgb24,
                        width * 3, width, height);
}
