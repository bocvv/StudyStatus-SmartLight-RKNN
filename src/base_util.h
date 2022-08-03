#ifndef BASE_UTIL_H
#define BASE_UTIL_H

#include <string>
#include <jni.h>

#define STATUS_MODEL_INPUT_WIDTH   256
#define STATUS_MODEL_INPUT_HEIGHT  224 
#define STATUS_CROP_WIDTH          592
#define STATUS_CROP_HEIGHT         518
#define STATUS_HEIGHT_START        98

#define LIGHT_MODEL_INPUT_WIDTH    256
#define LIGHT_MODEL_INPUT_HEIGHT   224
#define LIGHT_CROP_WIDTH           368
#define LIGHT_CROP_HEIGHT          322
#define LIGHT_HEIGHT_START         294

#define DVT2_FRAME_OUTPUT_SIZE     640

std::string getDirPath(JNIEnv *env, jstring dirPath);

std::string getFilePath(JNIEnv *env, jstring filePath);

std::string base64_encode(unsigned char const* bytes_to_encode, \
        unsigned int in_len);

std::string base64_decode(std::string const& encoded_string);

int compress_and_encode(cv::Mat img, std::string &enc_img_data);

int decode_base64_to_mat(std::string &enc_img_data, cv::Mat &img);

cv::Mat mat_rotate_clockwise_270(cv::Mat src);

cv::Mat preprocess(cv::Mat &rgb_image, int expect_width, int expect_height, \
        bool proc_for_status, bool use_dvt2 = true);

#endif //BASE_UTIL_H
