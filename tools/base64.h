#ifndef BASE_UTIL_H
#define BASE_UTIL_H

#include <string>

std::string base64_encode(unsigned char const* bytes_to_encode, \
        unsigned int in_len);

std::string base64_decode(std::string const& encoded_string);

int compress_and_encode(cv::Mat img, std::string &enc_img_data);

int decode_base64_to_mat(std::string &enc_img_data, cv::Mat &img);

#endif //BASE_UTIL_H
