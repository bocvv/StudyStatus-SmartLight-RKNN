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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#ifdef SS_DEBUG
#include <sys/time.h>
#endif
#include "study_status.h"
#include "base_util.h"
#include "timer_clock.h"
#ifdef USE_MODEL_ENCRYPT
#include "angouka_lite.hpp"
#endif

#include "version.h"

#define BACK_THRESHOLD          0.51
#define HEAD_THRESHOLD          0.4
#define ACTION_THRESHOLD        0.25
#define STATUS_FRAME_DEQUE_LENGTH  8
#define LIGHT_FRAME_DEQUE_LENGTH   4
#define BOOK_CLASS_NUM             2

#define SHADING_WIDTH_START     120
#define SHADING_WIDTH_END       520
#define SHADING_HEIGHT_START    60
#define SHADING_HEIGHT_END      580

// shading threshold
#define SHADING_THRESHOLD       25

// Timer of inference and deque's emplace_back()
#define NEXT_INFER_POINT  1000 // ms
#define SAMPLE_INTERVAL   333  // 2.6s/8


#define CHECK_NULL(x) 	do {\
				if (!(x))\
					return RKNN_ERR_PARAM_INVALID;\
			} while (0)

#define SIG_CANCEL_SIGNAL SIGUSR1

template < typename T > std::string to_string( const T& n )
{
    std::ostringstream stm ;
    stm << n ;
    return stm.str() ;
}
void read_frame_data(cv::Mat& yuv_mat, int frame_count) {
    std::string prefix = "/vendor/mpp_enc/test_frames/";
    std::string img_path = prefix + to_string(frame_count) + ".jpg";
    cv::Mat frame = cv::imread(img_path, 1);
    cv::cvtColor(frame, yuv_mat, cv::COLOR_BGR2YUV_I420);
}

template<class ForwardIterator>
inline size_t argmin(ForwardIterator first, ForwardIterator last) {
    return std::distance(first, std::min_element(first, last));
}

template<class ForwardIterator>
inline size_t argmax(ForwardIterator first, ForwardIterator last) {
    return std::distance(first, std::max_element(first, last));
}

static void print_rknn_tensor(rknn_tensor_attr *attr) {
    LOGI("index=%d name=%s n_dims=%d dims=[%d %d %d %d] n_elems=%d size=%d \ 
            fmt=%d type=%d qnt_type=%d fl=%d zp=%d scale=%f\n", attr->index, \
            attr->name, attr->n_dims, attr->dims[0], attr->dims[1], attr->dims[2], \
            attr->dims[3], attr->n_elems, attr->size, 0, attr->type, attr->qnt_type, \
            attr->fl, attr->zp, attr->scale);
}

static unsigned char* load_model(const char *filename, int *model_size)
{
    FILE *fp = fopen(filename, "rb");
    if(fp == nullptr) {
        LOGI("fopen %s fail!\n", filename);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);

    int model_len = ftell(fp);
    unsigned char *model = (unsigned char*)malloc(model_len);

    fseek(fp, 0, SEEK_SET);

    if(model_len != fread(model, 1, model_len, fp)) {
        LOGI("fread %s fail!\n", filename);
        free(model);
        return NULL;
    }

    *model_size = model_len;

    if(fp) {
        fclose(fp);
    }

#ifdef USE_MODEL_ENCRYPT
    // decrypt
    angouka_lite::mod_buffer((uint8_t*)model, model_len);
#endif

    return (unsigned char*)model;
}

int StudyStatusManager::rknn_model_init(unsigned char* &model, std::string model_path, \
        rknn_context &ctx, rknn_input_output_num &io_num) {
    int model_len{0};
    model = load_model(model_path.c_str(), &model_len);

    LOGI("rknn model %s init ...\n", model_path.c_str());
    int ret = rknn_init(&ctx, model, model_len, 0, NULL);
    if(ret < 0) {
        LOGI("rknn init fail! ret=%d\n", ret);
        return -1;
    }

    // Get Model Input Output Info
    LOGI("rknn model %s get Input Output Info ...\n", model_path.c_str());
    ret = get_ioput_info(ctx, io_num);
    if(ret != RKNN_SUCC) {
        LOGI("get model %s output info fail! ret = %d.\n", model_path.c_str(), ret);
        return -1;
    }

    return RKNN_SUCC;
}

int StudyStatusManager::get_ioput_info(rknn_context& ctx, rknn_input_output_num& io_num) {
    int ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret != RKNN_SUCC) {
        LOGI("rknn_query fail! ret=%d\n", ret);
        return ret;
    }
    LOGI("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

    LOGI("input tensors:\n");
    rknn_tensor_attr input_attrs[io_num.n_input];
    memset(input_attrs, 0, sizeof(input_attrs));
    for (int i = 0; i < io_num.n_input; i++) {
        input_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC) {
            LOGI("rknn_query fail! ret=%d\n", ret);
            return ret;
        }
        print_rknn_tensor(&(input_attrs[i]));
    }

    LOGI("output tensors:\n");
    rknn_tensor_attr output_attrs[io_num.n_output];
    memset(output_attrs, 0, sizeof(output_attrs));
    for (int i = 0; i < io_num.n_output; i++) {
        output_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC) {
            LOGI("rknn_query fail! ret=%d\n", ret);
            return ret;
        }
        print_rknn_tensor(&(output_attrs[i]));
    }

    return RKNN_SUCC;
}

void StudyStatusManager::set_imgproc_with_opencv(bool wanna_use_opencv) {
    use_opencv = wanna_use_opencv;
}


int StudyStatusManager::init(std::string conf_file_path, bool use_smart_light, bool use_dvt2) {
    // Load Config File Params
    conf = new Config(conf_file_path.c_str());
    data_dir = conf->Read("data_dir", std::string("./data"));
    camera_fps = conf->Read("camera_fps", 30.0);
    seq_length = conf->Read("seq_length", STATUS_FRAME_DEQUE_LENGTH);
    back_threshold = conf->Read("back_threshold", BACK_THRESHOLD);
    head_threshold = conf->Read("head_threshold", HEAD_THRESHOLD);
    action_threshold = conf->Read("action_threshold", ACTION_THRESHOLD);
    shading_threshold = conf->Read("shading_threshold", SHADING_THRESHOLD);
    // class number of every pose
    backpose_num = conf->Read("backpose_num", BACK_NUM);
    headpose_num = conf->Read("headpose_num", HEAD_NUM);
    actionpose_num = conf->Read("actionpose_num", ACTION_NUM);

    // calculate the inference params about time based on camera fps
    _use_smart_light = use_smart_light;
    _use_dvt2 = use_dvt2;

    LOGI("use_dvt2 = %d\n", _use_dvt2);
    LOGI("use_smart_light = %d\n", _use_smart_light);

    // init the StudyStatusResult res
    status_res = {.back = BackPose::upright,
           .head = HeadPose::up_head,
           .action = ActionPose::learn,
           .back_score = 1.0,  // Initialize. the max score value is 1.0, while the min is 0.
           .head_score = 1.0,
           .action_score = 1.0};
    light_res = SmartLightResult::textbook;

    // prepare the input_data_buf's memory
    one_status_frame_size = STATUS_MODEL_INPUT_WIDTH * STATUS_MODEL_INPUT_HEIGHT * 3;
    one_light_frame_size = LIGHT_MODEL_INPUT_WIDTH * LIGHT_MODEL_INPUT_HEIGHT * 3;

    // Load RKNN Model
    LOGI("Loading model ...\n");
    std::string model_name = conf->Read("study_status_model_name", std::string("status_model.rknn"));
    std::string model_path = data_dir + "/" + model_name;
    int ret = rknn_model_init(status_model, model_path, status_ctx, status_io_num);
    if (ret < 0) {
        LOGI("rknn model init fail! ret = %d.\n", ret);
        return ret;
    }

    if (_use_smart_light) {
        model_name = conf->Read("smart_light_model_name", std::string("light_model.rknn"));
        model_path = data_dir + "/" + model_name;
        ret = rknn_model_init(light_model, model_path, light_ctx, light_io_num);
        if (ret < 0) {
            LOGI("rknn model init fail! ret = %d.\n", ret);
            return ret;
        }
    }

    // init aladdin media encoder and log module
    std::string module_name = "LearnStatus";
    std::string module_version = LIB_VERSION;
    std::string sdk_name = "StatusRec";
    std::string sdk_version = LIB_VERSION;
    if (_use_smart_light) {
        module_name = "SmartLight";
        sdk_name = "LightRec";
    }
#ifdef USE_MEDIA_ENCODER
    m_mgr = new MediaManager(module_name.c_str(), module_version.c_str(), \
               sdk_name.c_str(), sdk_version.c_str());
#endif

    LOGI("Init Succeed!!");

    return 0;
}

StudyStatusManager::StudyStatusManager(std::string conf_file_path, \
        bool use_smart_light, bool use_dvt2, int &ret) {
    ret = init(conf_file_path, use_smart_light, use_dvt2);
    t_frame_deque = new std::thread(&StudyStatusManager::update_frame_deque, this);
    t_frame_deque->detach();
    t_study_status = new std::thread(&StudyStatusManager::update_study_status, this);
    t_study_status->detach();
}

void StudyStatusManager::update_curr_status(float* predictions) {
    int back_idx = argmax(predictions, predictions + backpose_num);
    int head_idx = argmax(predictions + backpose_num, \
            predictions + backpose_num + headpose_num);
    int action_idx = argmax(predictions + backpose_num + headpose_num, \
            predictions + backpose_num + headpose_num + actionpose_num);

    status_res.back_score = predictions[back_idx];
    status_res.head_score = predictions[backpose_num + head_idx];
    status_res.action_score = predictions[backpose_num + headpose_num + action_idx];

    if (status_res.back_score > back_threshold) {
        status_res.back = static_cast<BackPose>(back_idx);
    } else {
        status_res.action = ActionPose::nobody;
    }
    if (status_res.head_score > head_threshold) {
        status_res.head = static_cast<HeadPose>(head_idx);
    } else {
        status_res.action = ActionPose::nobody;
    }
    if (status_res.action_score > action_threshold) {
        status_res.action = static_cast<ActionPose>(action_idx);
    }

    return;
}

void StudyStatusManager::update_curr_light(float* predictions) {
    int class_id = argmax(predictions, predictions + BOOK_CLASS_NUM);

    light_res = (class_id == 0) ? SmartLightResult::textbook : \
                SmartLightResult::cartoon;

    light_score = predictions[class_id];

    return;
}

int StudyStatusManager::inference() {
    // Avoid that deque's length is less than STATUS_FRAME_DEQUE_LENGTH frame.
    if (frame_deque.size() != STATUS_FRAME_DEQUE_LENGTH) {
        LOGI("rknn inference fail! Frame deque's length is not STATUS_FRAME_DEQUE_LENGTH(=%d).", \
                STATUS_FRAME_DEQUE_LENGTH);
        return -1;
    }

    // Set Model's Input Data
    int deque_idx{0};
    rknn_input inputs[STATUS_FRAME_DEQUE_LENGTH];
    memset(inputs, 0, sizeof(inputs));

    for (std::deque<cv::Mat>::const_iterator it = frame_deque.begin(); \
            it != frame_deque.end(); it++) {
        cv::Mat iter_mat = *it;

        inputs[deque_idx].index = deque_idx;
        inputs[deque_idx].type = RKNN_TENSOR_UINT8;
        inputs[deque_idx].size = one_status_frame_size;
        inputs[deque_idx].fmt = RKNN_TENSOR_NHWC;
        inputs[deque_idx].buf = iter_mat.data;
        inputs[deque_idx].pass_through = false;

        ++deque_idx;
    }

    int ret = rknn_inputs_set(status_ctx, status_io_num.n_input, inputs);
    if(ret < 0) {
        LOGI("rknn_input_set fail! ret= %d.\n", ret);
        return ret;
    }

    // Run the Inference once
    LOGI("study status rknn_run.\n");

#ifdef SS_DEBUG
    struct timeval t = {0, 0};
    gettimeofday(&t, nullptr);
    double start_time = t.tv_sec + t.tv_usec * 1e-6;
#endif

    ret = rknn_run(status_ctx, nullptr);
    
#ifdef SS_DEBUG
    gettimeofday(&t, nullptr);
    double end_time = t.tv_sec + t.tv_usec * 1e-6;
    LOGI("Study Status RKNN Model Run Time = %f ms.\n", (end_time - start_time) * 1000);
#endif

    if(ret < 0) {
        LOGI("study status rknn_run fail! ret= %d.\n", ret);
        return ret;
    }

    // Get Output
    rknn_output outputs[1];
    memset(outputs, 0, sizeof(outputs));
    outputs[0].want_float = 1;
    ret = rknn_outputs_get(status_ctx, status_io_num.n_output, outputs, NULL);
    if(ret < 0) {
        LOGI("study status rknn_outputs_get fail! ret= %d.\n", ret);
        return ret;
    }

    // update the StudyStatusResult
    update_curr_status((float *)(outputs[0].buf));

    // Release rknn_outputs
    rknn_outputs_release(status_ctx, status_io_num.n_output, outputs);

    return 0;
}

void StudyStatusManager::update_frame_deque() {
    // push current frame to the queue and decide whether to do inference,
    // current frame need to be push into the frame_deque
    cv::Mat rgb_image(cap_height, cap_width, CV_8UC3);  // with RGB format
    cv::Mat smart_light_image;
    cv::Mat preprocess_image;

    /*
    while (yuv_mat.empty()) {
        // sleep_for(10ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    */

    TimerClock tc;
    while (!is_deque_stop) {
        tc.update();

        // Preprocess the Input Image
#ifdef SS_DEBUG
        struct timeval t = {0, 0};
        gettimeofday(&t, nullptr);
        double start_time = t.tv_sec + t.tv_usec * 1e-6;
#endif

        {
            std::lock_guard<std::mutex> g_lock(mtx_frame);
            if (yuv_mat.empty()) {
                continue;  // Not elegant, but works temporarily.
            }
            cv::cvtColor(yuv_mat, rgb_image, cv::COLOR_YUV2RGB_NV21);
        }

        if (cap_height != DVT2_FRAME_OUTPUT_SIZE || cap_height != cap_width) {
            cv::resize(rgb_image, rgb_image, cv::Size(DVT2_FRAME_OUTPUT_SIZE, DVT2_FRAME_OUTPUT_SIZE), \
                    (0, 0), (0, 0), cv::INTER_AREA);
        }
        rgb_image = mat_rotate_clockwise_270(rgb_image);
        preprocess_image = preprocess(rgb_image, STATUS_MODEL_INPUT_WIDTH, STATUS_MODEL_INPUT_HEIGHT, true);
        
        {
            std::lock_guard<std::mutex> g_lock(mtx_frame);

            frame_deque.emplace_back(preprocess_image);
            // calculate the shading value of one single rgb image with 640x640 pisxels
            shading_deque.emplace_back(static_cast<int>(is_single_shading(rgb_image)));
#ifdef SS_DEBUG
            LOGI("_need_save_light_frames = %d.\n", _need_save_light_frames);
#endif
            if (_use_smart_light && _need_save_light_frames) {
                smart_light_image = preprocess(rgb_image, LIGHT_MODEL_INPUT_WIDTH, LIGHT_MODEL_INPUT_HEIGHT, false);
                smart_light_deque.emplace_back(smart_light_image);
            }

#ifdef SS_DEBUG
            gettimeofday(&t, nullptr);
            double end_time = t.tv_sec + t.tv_usec * 1e-6;
            LOGI("total preprocess & empace_back Time = %f ms\n", (end_time - start_time) * 1000);
#endif

            if (frame_deque.size() > STATUS_FRAME_DEQUE_LENGTH) {
                // firstly pop the head past frame, then STATUS_FRAME_DEQUE_LENGTH frames left
                frame_deque.pop_front();
                shading_deque.pop_front();
            }
            LOGI("frame_deque.size = %d\n", frame_deque.size());

            // when _use_smart_light is false, we should never come to this branch.
            if (_use_smart_light && _need_save_light_frames) {
                if (smart_light_deque.size() > LIGHT_FRAME_DEQUE_LENGTH) {
                    smart_light_deque.pop_front();
                }
            }
#ifdef SS_DEBUG
            LOGI("smart_light_deque.size = %d\n", smart_light_deque.size());
#endif
            _need_save_light_frames = !_need_save_light_frames;
        }

        // sleep for (2.6/8=0.325s)
        int fixed_sample_interval = SAMPLE_INTERVAL - tc.get_timer_millisec();

#ifdef SS_DEBUG
        LOGI("fixed_sample_interval = %d", fixed_sample_interval);
#endif

        // if user want to stop the study status, then do not wait for another period.
        if (fixed_sample_interval > 0 && !is_deque_stop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(fixed_sample_interval));
        }
    }

    t_update_frame_deque_finished = true;
}

void StudyStatusManager::update_study_status() {
    cv::Mat rgb_image(cap_height, cap_width, CV_8UC3);  // with RGB format
    cv::Mat smart_light_image;
    cv::Mat preprocess_image;
    int wait_time;

    /*
    while (frame_deque.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    */

    // sleep for (1500ms) and wait for 8-frames pushed back; -100 to emplace the other steps's time spent.
    std::this_thread::sleep_for(std::chrono::milliseconds(NEXT_INFER_POINT));

    TimerClock tc;
    // need execute inference at checkpoint T and T+1.5
    while (!is_status_stop) {
        tc.update();

        // the condition that frame queue's size is not enough
#ifdef SS_DEBUG
        LOGI("ss_frame_deque.size = %d", frame_deque.size());
#endif

        if (frame_deque.size() < STATUS_FRAME_DEQUE_LENGTH) {
            {
                std::lock_guard<std::mutex> g_lock(mtx_frame);

                if (frame_deque.empty()) {
                    continue;
                }

                int empty_queue_num = STATUS_FRAME_DEQUE_LENGTH - frame_deque.size();

                if (yuv_mat.empty()) {
                    continue;  // Not elegant, but works temporarily.
                }
                cv::cvtColor(yuv_mat, rgb_image, cv::COLOR_YUV2RGB_NV21);

                if (cap_height != DVT2_FRAME_OUTPUT_SIZE || cap_height != cap_width) {
                    cv::resize(rgb_image, rgb_image, cv::Size(DVT2_FRAME_OUTPUT_SIZE, DVT2_FRAME_OUTPUT_SIZE), \
                            (0, 0), (0, 0), cv::INTER_AREA);
                }
                rgb_image = mat_rotate_clockwise_270(rgb_image);
                preprocess_image = preprocess(rgb_image, STATUS_MODEL_INPUT_WIDTH, STATUS_MODEL_INPUT_HEIGHT, true);

                // Completion of empty frames in the deque
                for (int i = 0; i < empty_queue_num; i++) {
                    frame_deque.emplace_back(preprocess_image);
                }

                int ret = inference();
                if(ret < 0) {
                    LOGI("rknn inference fail! ret=%d\n", ret);
                }

                // update shading value with shading_deque.size()'s images, the deque's size must larger than one.
                shading_deque.emplace_back(static_cast<int>(is_single_shading(rgb_image)));
                update_curr_shading();
                shading_deque.pop_back();

                // Delete the frame populated in the deque
                for (int i = 0; i < empty_queue_num; i++) {
                    frame_deque.pop_back();
                }

                // do the inference of smart light result
                if (_use_smart_light) {
                    empty_queue_num = LIGHT_FRAME_DEQUE_LENGTH - smart_light_deque.size();
                    smart_light_image = preprocess(rgb_image, LIGHT_MODEL_INPUT_WIDTH, LIGHT_MODEL_INPUT_HEIGHT, false);
                    for (int i = 0; i < empty_queue_num; i++) {
                        smart_light_deque.emplace_back(smart_light_image);
                    }
                    switch(status_res.action) {
                        case ActionPose::learn : {
                            int ret = smart_light_inference();
                            if (ret < 0) {
                                LOGI("not enough deque size's smart light inference fail! ret = %d\n.", ret);
                            }
                            break;
                        }
                        case ActionPose::pad : case ActionPose::computer : case ActionPose::phone : {
                            light_res = SmartLightResult::online_class;
                            break;
                        }
                        case ActionPose::sleep : {
                            light_res = SmartLightResult::relax;
                            break;
                        }
                        default: {
                            light_res = SmartLightResult::ignore;
                        }
                    }
                    for (int i = 0; i < empty_queue_num; i++) {
                        smart_light_deque.pop_back();
                    }
                }
            }

            wait_time = NEXT_INFER_POINT;
            int fixed_wait_time = wait_time - tc.get_timer_millisec();

#ifdef SS_DEBUG
            LOGI("not_enough_fixed_wait_time = %d", fixed_wait_time);
#endif
            // if user want to stop the study status, then do not wait for another period.
            if (fixed_wait_time > 0 && !is_status_stop) {
                std::this_thread::sleep_for(std::chrono::milliseconds(fixed_wait_time));
            }
            
            continue;
        }

        {
            // Because of mutex, frame_deque.size() will be STATUS_FRAME_DEQUE_LENGTH exactly here.
            std::lock_guard<std::mutex> g_lock(mtx_frame);

            // execute the inference & update the res value
            int ret = inference();
            if(ret < 0) {
                LOGI("rknn study status model inference fail! ret=%d\n", ret);
            }
            update_curr_shading();

            if (_use_smart_light) {
                switch(status_res.action) {
                    case ActionPose::learn : {
                        int ret = smart_light_inference();
                        if (ret < 0) {
                            LOGI("smart light inference fail! ret = %d\n.", ret);
                        }
                        break;
                    }
                    case ActionPose::pad : case ActionPose::computer : case ActionPose::phone : {
                        light_res = SmartLightResult::online_class;
                        break;
                    }
                    case ActionPose::sleep : {
                        light_res = SmartLightResult::relax;
                        break;
                    }
                    default: {
                        light_res = SmartLightResult::ignore;
                    }
                }
            }

        }

        wait_time = NEXT_INFER_POINT;
        int fixed_wait_time = wait_time - tc.get_timer_millisec();
        LOGI("8frames_fixed_wait_time = %d", fixed_wait_time);
        // if user want to stop the study status, then do not wait for another period.
        if (fixed_wait_time > 0 && !is_status_stop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(fixed_wait_time));
        }
    }

    t_update_study_status_finished = true;
}

StudyStatusResult StudyStatusManager::get_study_status(cv::Mat &yuv_image, int width, int height) {
    cap_width = width;
    cap_height = height;
    LOGI("before log push");

    {
        std::lock_guard<std::mutex> g_lock(mtx_frame);
        yuv_mat = yuv_image;
        has_input_data = true;
#ifdef USE_MEDIA_ENCODER
        if (log_frame_idx % log_interval == 0) {
            log_frame_idx = 0;
        }

        if (log_frame_idx == log_start_point) {
            m_mgr->init(width, height, FMT_YUV420SP_VU, VIDEO_CodingHEVC, log_period, 23); // encode 800 frames@24fps to one h265 file
            LOGI("create new video log cxt succeed!");
        }

        if (log_frame_idx >= log_start_point && log_frame_idx < log_end_point) {
            m_mgr->run(yuv_mat.data);
        } else if (log_frame_idx == log_end_point) {
            LOGI("start to send recent video log, then free its' resource.");
            if (m_mgr->get_total_len() != 0) {
                m_mgr->send_log(1);  // 1 means log type of video
                LOGI("send video log succeed!");
                m_mgr->close();
                LOGI("close old video log cxt succeed!");
            }
        }

        ++log_frame_idx;
#endif
    }

    return status_res;
}

int StudyStatusManager::smart_light_inference() {
    // Avoid that deque's length is less than LIGHT_FRAME_DEQUE_LENGTH frame.
    if (smart_light_deque.size() != LIGHT_FRAME_DEQUE_LENGTH) {
        LOGI("rknn inference fail! SmartLight frame deque's length is not LIGHT_FRAME_DEQUE_LENGTH(=%d).", \
                LIGHT_FRAME_DEQUE_LENGTH);
        return -1;
    }

    // Set Model's Input Data
    int deque_idx{0};
    rknn_input inputs[LIGHT_FRAME_DEQUE_LENGTH];
    memset(inputs, 0, sizeof(inputs));

    for (std::deque<cv::Mat>::const_iterator it = smart_light_deque.begin(); \
            it != smart_light_deque.end(); it++) {
        cv::Mat iter_mat = *it;

        inputs[deque_idx].index = deque_idx;
        inputs[deque_idx].type = RKNN_TENSOR_UINT8;
        inputs[deque_idx].size = one_light_frame_size;
        inputs[deque_idx].fmt = RKNN_TENSOR_NHWC;
        inputs[deque_idx].buf = iter_mat.data;
        inputs[deque_idx].pass_through = false;

        ++deque_idx;
    }

    int ret = rknn_inputs_set(light_ctx, light_io_num.n_input, inputs);
    if(ret < 0) {
        LOGI("rknn_input_set fail! ret= %d.\n", ret);
        return ret;
    }

    // Run the Inference once
    LOGI("smart light rknn_run.\n");

#ifdef SS_DEBUG
    struct timeval t = {0, 0};
    gettimeofday(&t, nullptr);
    double start_time = t.tv_sec + t.tv_usec * 1e-6;
#endif

    ret = rknn_run(light_ctx, nullptr);
    
#ifdef SS_DEBUG
    gettimeofday(&t, nullptr);
    double end_time = t.tv_sec + t.tv_usec * 1e-6;
    LOGI("Smart Light RKNN Model Run Time = %f ms.\n", (end_time - start_time) * 1000);
#endif

    if(ret < 0) {
        LOGI("smart light rknn_run fail! ret= %d.\n", ret);
        return ret;
    }

    // Get Output
    rknn_output outputs[1];
    memset(outputs, 0, sizeof(outputs));
    outputs[0].want_float = 1;
    ret = rknn_outputs_get(light_ctx, light_io_num.n_output, outputs, NULL);
    if(ret < 0) {
        LOGI("smart light rknn_outputs_get fail! ret= %d.\n", ret);
        return ret;
    }

    // update the SmartLightResult
    update_curr_light((float *)(outputs[0].buf));

    // Release rknn_outputs
    rknn_outputs_release(light_ctx, light_io_num.n_output, outputs);

    return 0;
}

bool StudyStatusManager::is_single_shading(cv::Mat rgb_image) {
    cv::Mat cropped_image = rgb_image(cv::Range(SHADING_HEIGHT_START, SHADING_HEIGHT_END), \
            cv::Range(SHADING_WIDTH_START, SHADING_WIDTH_END));

    // use for calculate the shading value.
    cv::Mat gray_mat;
    cv::Mat mat_mean;
    cv::Mat mat_stddev;

    // calculate the mean gray-scale value of each mat in the deque.
    cv::cvtColor(cropped_image, gray_mat, cv::COLOR_RGB2GRAY);
    cv::meanStdDev(gray_mat, mat_mean, mat_stddev);
    LOGI("curr image's grayscale stderr = %f.\n", mat_stddev.at<double>(0, 0));

    shading_mean = mat_mean.at<double>(0, 0);
    shading_stddev = mat_stddev.at<double>(0, 0);
    if (shading_stddev < shading_threshold) {
        return true;
    }

    return false;
}

void StudyStatusManager::update_curr_shading() {
    /*
     * old strategy that sum the 'shading status' number in one period. *
    int shading_sum(0);
    for (std::deque<int>::const_iterator it = shading_deque.begin(); \
            it != shading_deque.end(); it++) {
        shading_sum += *it;
    }
    if (shading_sum > 0.75 * shading_deque.size()) {
        curr_shading_flag = true;
    } else {
        curr_shading_flag = false;
    }
    */
    if (shading_deque.back() == 1) {
        curr_shading_flag = true;
    } else {
        curr_shading_flag = false;
    }

    return;
}

bool StudyStatusManager::get_shading() {
    return curr_shading_flag;
}

float StudyStatusManager::get_shading_mean() {
    return shading_mean;
}

float StudyStatusManager::get_shading_stddev() {
    return shading_stddev;
}

SmartLightResult StudyStatusManager::get_smart_light(cv::Mat &yuv_image, int width, int height) {
    StudyStatusResult tmp_status_res = get_study_status(yuv_image, width, height);

    return light_res;
}

StudyStatusResult StudyStatusManager::get_curr_study_status_result() {
    return status_res;
}

int StudyStatusManager::release() {
    if(status_ctx >= 0) {
        rknn_destroy(status_ctx);
    }
    if(_use_smart_light && light_ctx >= 0) {
        rknn_destroy(light_ctx);
    }

    if(status_model) {
        free(status_model);
    }
    if(_use_smart_light && light_model) {
        free(light_model);
    }

    return 0;
}

StudyStatusManager::~StudyStatusManager() {
    delete conf;
#ifdef USE_MEDIA_ENCODER
    delete m_mgr;
#endif
    is_status_stop = true;
    while (1) {
        if (t_update_study_status_finished) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    is_deque_stop = true;
    while (1) {
        if (t_update_frame_deque_finished) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // pthread_kill(t_deque_handle, SIG_CANCEL_SIGNAL);
    // pthread_kill(t_status_handle, SIG_CANCEL_SIGNAL);

    release();
}

