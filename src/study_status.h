/*****************************************************************************/
/* Copyright YouDao, Inc.                                                    */
/*                                                                           */
/* Licensed under the Apache License, Version 2.0 (the "License");           */
/* you may not use this file except in compliance with the License.          */
/* You may obtain a copy of the License at                                   */
/*                                                                           */
/*     http://www.apache.org/licenses/LICENSE-2.0                            */
/*                                                                           */
/* Unless required by applicable law or agreed to in writing, software       */
/* distributed under the License is distributed on an "AS IS" BASIS,         */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  */
/* See the License for the specific language governing permissions and       */
/* limitations under the License.                                            */
/*****************************************************************************/

#ifndef STUDY_STATUS_H_
#define STUDY_STATUS_H_

#include <string>
#include <deque>
#include <thread>
#include <mutex>

#include "rknn_api.h"
#include "config/cls_config.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

#ifdef USE_MEDIA_ENCODER
#include "media_manager.h"
#endif

#include <android/log.h>
#define LOG_TAG "StudyStatus"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define BACK_NUM            3
#define HEAD_NUM            5
#define ACTION_NUM          9

#define CAP_WIDTH           1536
#define CAP_HEIGHT          1536

enum class BackPose {
    upright                = 0,
    hump                   = 1,
    other_sitting_posture  = 2
};

enum class HeadPose {
    up_head                = 0,
    tilt_head              = 1,
    hold_head              = 2,
    grovel                 = 3,
    other_head_posture     = 4
};

enum class ActionPose {
    learn                  = 0,
    look_around            = 1,
    pad                    = 2,
    computer               = 3,
    phone                  = 4,
    playing                = 5,
    eating_drinking        = 6,
    sleep                  = 7,
    nobody                 = 8
};

enum class SmartLightResult {
    textbook               = 0,
    cartoon                = 1,
    online_class           = 2,
    relax                  = 3,
    ignore                 = 4
};

struct StudyStatusResult {
    BackPose back;
    HeadPose head;
    ActionPose action;
    float back_score;
    float head_score;
    float action_score;
};

class StudyStatusManager {
public:
    StudyStatusManager() {}
    StudyStatusManager(std::string conf_file_path, bool use_smart_light, bool use_dvt2, int &ret);
    ~StudyStatusManager();

    StudyStatusResult get_study_status(cv::Mat &yuv_image, int width, int height);
    SmartLightResult get_smart_light(cv::Mat &yuv_image, int width, int height);
    StudyStatusResult get_curr_study_status_result();
    bool get_shading();
    float get_shading_mean();
    float get_shading_stddev();
    void set_imgproc_with_opencv(bool wanna_use_opencv);

private:
    int init(std::string conf_file_path, bool use_smart_light = false, bool use_dvt2 = true);
    int release();
    int get_ioput_info(rknn_context& ctx, rknn_input_output_num& io_num);
    void update_curr_status(float* predictions);
    int inference();
    int rknn_model_init(unsigned char* &model, std::string model_path, \
            rknn_context &ctx, rknn_input_output_num &io_num);
    void update_curr_light(float* predictions);
    int smart_light_inference();
    bool is_single_shading(cv::Mat rgb_image);
    void update_curr_shading();
    void update_frame_deque();
    void update_study_status();

    // params in config file which can be set by users
    Config *conf{nullptr};
    float camera_fps;
    int seq_length;
    int backpose_num;  // class number of backpose
    int headpose_num;  // class number of headpose
    int actionpose_num;  // class number of actionpose
    float back_threshold;
    float head_threshold;
    float action_threshold;
    std::string data_dir;
    
    // rknn model used to be inference
    unsigned char* status_model{nullptr};
    rknn_context status_ctx;
    rknn_input_output_num status_io_num;

    // smart light's params
    bool _use_smart_light{false};
    bool _need_save_light_frames{false};
    bool _use_dvt2{false};
    unsigned char* light_model{nullptr};
    rknn_context light_ctx;
    rknn_input_output_num light_io_num;

    // video&camera frame deque and its' related params
    std::deque<cv::Mat> frame_deque;
    std::deque<cv::Mat> smart_light_deque;
    std::deque<int> shading_deque;
    int cap_width{CAP_WIDTH};
    int cap_height{CAP_HEIGHT};
    cv::Mat yuv_mat;

    // infrence's input 8-frame data
    int one_status_frame_size;
    int one_light_frame_size;
    unsigned char* input_data_buf;

    // detection result of our model
    StudyStatusResult status_res;
    SmartLightResult light_res;
    float light_score{0.0};
    
    // shading status value
    bool curr_shading_flag{false};
    float shading_mean{255.0};
    float shading_stddev{255.0};
    float shading_threshold;

    // thread that saves the current input frame data.
    std::thread* t_frame_deque{nullptr};
    std::mutex mtx_frame;
    // thread that managers the frame deque's pop and push.
    std::thread* t_study_status{nullptr};
    std::mutex mtx_deque;
    volatile bool is_deque_stop{false};
    volatile bool is_status_stop{false};
    volatile bool t_update_frame_deque_finished{false};
    volatile bool t_update_study_status_finished{false};

    // var of contoling the data input format
    bool use_opencv{true};  // if use libyuv, then set it false.
    volatile bool has_input_data{false};  // if start push data, then set it true.

#ifdef USE_MEDIA_ENCODER
    // Aladdin Log Params
    MediaManager* m_mgr{nullptr};
    int log_frame_idx{0};
    int log_interval{20800};  // about 15min
    int log_start_point{4200};     // at 3min
    int log_end_point{5800};     // at 4min
    int log_period{1600};    // about 1min
#endif
};

#endif
