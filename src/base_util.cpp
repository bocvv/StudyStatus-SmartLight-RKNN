#include <sys/time.h>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "base_util.h"

#include <android/log.h>
#define LOG_TAG "BaseUtil"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string getDirPath(JNIEnv *env, jstring dirPath) {
    if (NULL == dirPath) {
        return nullptr;
    }
    const char *file_path = env->GetStringUTFChars(dirPath, 0);
    if (NULL == file_path) {
        return nullptr;
    }
    std::string t_file_path = file_path;
    std::string tLastChar = t_file_path.substr(t_file_path.length() - 1, 1);
    //目录补齐/
    if ("\\" == tLastChar) {
        t_file_path = t_file_path.substr(0, t_file_path.length() - 1) + "/";
    } else if (tLastChar != "/") {
        t_file_path += "/";
    }
    return t_file_path;
}

std::string getFilePath(JNIEnv *env, jstring filePath) {
    if (NULL == filePath) {
        return nullptr;
    }
    const char *file_path = env->GetStringUTFChars(filePath, 0);
    if (NULL == file_path) {
        return nullptr;
    }
    return file_path;
}

std::string base64_encode(unsigned char const* bytes_to_encode, \
        unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

std::string base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

int compress_and_encode(cv::Mat img, std::string &enc_img_data) {
  std::vector<unsigned char> buf;
  std::vector<int> quality;
  quality.push_back(cv::IMWRITE_JPEG_QUALITY);
  quality.push_back(25);
  cv::imencode(".jpg", img, buf, quality);
  auto *enc_msg = reinterpret_cast<const unsigned char*>(buf.data());
  enc_img_data = base64_encode(enc_msg, buf.size());
  return 0;
}

int decode_base64_to_mat(std::string &enc_img_data, cv::Mat &img) {
  std::string dec_jpg = base64_decode(enc_img_data);
  std::vector<unsigned char> data(dec_jpg.begin(), dec_jpg.end());
  img = cv::imdecode(cv::Mat(data), 1);
  return 0;
}

cv::Mat mat_rotate_clockwise_270(cv::Mat src) {
    if (src.empty()) {
        return src;
    }

    // 矩阵转置
    cv::transpose(src, src);
    //0: 沿X轴翻转； >0: 沿Y轴翻转； <0: 沿X轴和Y轴翻转
    cv::flip(src, src, 0);
    return src;
}

cv::Mat preprocess_original_version(cv::Mat &image, int width, int height, \
        int expect_width, int expect_height) {

    // resize origin image to with Size(expect_width, expect_height)
    int origin_w = image.cols;
    int origin_h = image.rows;
    int h_resized = expect_height;
    int w_resized = expect_width;

    if (origin_h > expect_height || origin_w > expect_width) {
        if (origin_h * 1.0 / origin_w >= expect_height * 1.0 / expect_width) {
            w_resized = (int)(origin_w * (expect_height * 1.0 / origin_h));
        }
        else {
            h_resized = (int)(origin_h * (expect_width * 1.0 / origin_w));
        }
    }

#ifdef SS_DEBUG
    struct timeval t = {0, 0};
    gettimeofday(&t, nullptr);
    double start_time = t.tv_sec + t.tv_usec * 1e-6;
#endif

    cv::Mat image_resized;
    // if origin input image needs to rotate, may need to exchange h_resized and w_resized
    cv::resize(image, image_resized, cv::Size(w_resized, h_resized), \
            (0, 0), (0, 0), cv::INTER_AREA);

#ifdef SS_DEBUG
    gettimeofday(&t, nullptr);
    double end_time = t.tv_sec + t.tv_usec * 1e-6;
    LOGI("preprocess resize TIME = %f ms.\n", (end_time - start_time) * 1000);
#endif

    // padding if required
    int pad_up = (expect_height - h_resized) / 2;
    int pad_bottom = expect_height - pad_up - h_resized;
    int pad_left = (expect_width - w_resized) / 2;
    int pad_right = expect_width - pad_left - w_resized;

#ifdef SS_DEBUG
    gettimeofday(&t, nullptr);
    start_time = t.tv_sec + t.tv_usec * 1e-6;
#endif

    cv::Mat image_pad;
    cv::copyMakeBorder(image_resized, image_pad, pad_up, pad_bottom, pad_left, \
            pad_right, cv::BORDER_CONSTANT, cv::Scalar(0));

#ifdef SS_DEBUG
    gettimeofday(&t, nullptr);
    end_time = t.tv_sec + t.tv_usec * 1e-6;
    LOGI("preprocess padding TIME = %f ms.\n", (end_time - start_time) * 1000);
#endif

    return image_pad;
}

int handmachine_preprocess_status(cv::Mat &rgb_image, cv::Mat &processed_image, \
        int expect_width, int expect_height) {
    return 0;
}

int handmachine_preprocess_light(cv::Mat &rgb_image, cv::Mat &processed_image, \
        int expect_width, int expect_height) {
    return 0;
}

int handmachine_preprocess(cv::Mat &rgb_image, cv::Mat &processed_image, \
        int expect_width, int expect_height, bool proc_for_status) {
    int ret{0};

    if (proc_for_status) {  // process for study status
        ret = handmachine_preprocess_status(rgb_image, processed_image, \
                expect_width, expect_height);
    } else {  // process for smart light
        ret = handmachine_preprocess_light(rgb_image, processed_image, \
                expect_width, expect_height);
    }
    if (ret < 0) {
        LOGI("handmachine preprocess inputed image with error! ret = %d.\n", ret);
    }

    return ret;
}

int dvt2_preprocess_status(cv::Mat &rgb_image, cv::Mat &processed_image, \
        int expect_width, int expect_height) {
    int origin_w = rgb_image.cols;
    int origin_h = rgb_image.rows;

    int w_start = (origin_w - STATUS_CROP_WIDTH) / 2;
    int w_end = (origin_w + STATUS_CROP_HEIGHT) / 2;
    int h_start = STATUS_HEIGHT_START;
    int h_end = h_start + STATUS_CROP_HEIGHT;

    cv::Mat cropped_image = rgb_image(cv::Range(h_start, h_end), cv::Range(w_start, w_end));
    cv::resize(cropped_image, processed_image, cv::Size(expect_width, \
            expect_height), (0, 0), (0, 0), cv::INTER_AREA);

    return 0;
}

int dvt2_preprocess_light(cv::Mat &rgb_image, cv::Mat &processed_image, \
        int expect_width, int expect_height) {
    int origin_w = rgb_image.cols;
    int origin_h = rgb_image.rows;

    int w_start = (origin_w - LIGHT_CROP_WIDTH) / 2;
    int w_end = (origin_w + LIGHT_CROP_HEIGHT) / 2;
    int h_start = LIGHT_HEIGHT_START;
    int h_end = h_start + LIGHT_CROP_HEIGHT;

    cv::Mat cropped_image = rgb_image(cv::Range(h_start, h_end), cv::Range(w_start, w_end));
    cv::resize(cropped_image, processed_image, cv::Size(expect_width, \
            expect_height), (0, 0), (0, 0), cv::INTER_AREA);

    return 0;
}

int dvt2_preprocess(cv::Mat &rgb_image, cv::Mat &processed_image, \
        int expect_width, int expect_height, bool proc_for_status) {
    int ret{0};

    if (proc_for_status) {  // process for study status
        ret = dvt2_preprocess_status(rgb_image, processed_image, \
                expect_width, expect_height);
    } else {  // process for smart light
        ret = dvt2_preprocess_light(rgb_image, processed_image, \
                expect_width, expect_height);
    }
    if (ret < 0) {
        LOGI("dvt2 preprocess inputed image with error! ret = %d.\n", ret);
    }

    return ret;
}

cv::Mat preprocess(cv::Mat &rgb_image, int expect_width, int expect_height, bool proc_for_status, bool use_dvt2) {
    cv::Mat image;
    cv::Mat processed_image;
    int ret{0};

    if (use_dvt2) {
        ret = dvt2_preprocess(rgb_image, processed_image, expect_width, \
                expect_height, proc_for_status);
    } else {  // use shouban_machine
        ret = handmachine_preprocess(rgb_image, processed_image, expect_width, \
                expect_height, proc_for_status);
    }
    if (ret < 0) {
        LOGI("preprocess inputed image with error! ret = %d.\n", ret);
    }

    return processed_image;
}
