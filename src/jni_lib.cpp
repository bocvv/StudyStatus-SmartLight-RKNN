#include "study_status.h"
#include "base_util.h"

#include <jni.h>
#include <string>

#define CLASS_NUM 3
#define CAP_WIDTH  1536
#define CAP_HEIGHT 1536

StudyStatusManager *ss_mgr{nullptr};
static jbyte *src_i420_data = nullptr;
std::mutex g_mtx;

template < typename T > std::string to_string( const T& n )
{
    std::ostringstream stm ;
    stm << n ;
    return stm.str() ;
}

/**
 *  description: Create a model which will be used to infer the study status
 *    result with input image.
 *  @param configFilePath: the path of onfig file, saving all the related params.
 *
 *  @return: true for initializing successfully, while failure with false.
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_com_youdao_ocr_StudyStatus_init(
        JNIEnv* env,
        jobject /* this */,
        jstring configFilePath,
        jboolean useSmartLight,
        jboolean useDVT2) {
    std::lock_guard<std::mutex> guard(g_mtx);

    std::string confPath = getFilePath(env, configFilePath);
    int ret;
    ss_mgr = new StudyStatusManager(confPath.c_str(), useSmartLight, useDVT2, ret);
    if (ret < 0) {
        LOGI("jni init fail!\n");
        return false;
    }
    ss_mgr->set_imgproc_with_opencv(true);

    return true;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_youdao_ocr_AladdinStudyStatus_init(
        JNIEnv* env,
        jobject /* this */,
        jstring configFilePath,
        jboolean useSmartLight,
        jboolean useDVT2) {
    std::lock_guard<std::mutex> guard(g_mtx);

    std::string confPath = getFilePath(env, configFilePath);
    int ret;
    ss_mgr = new StudyStatusManager(confPath.c_str(), useSmartLight, useDVT2, ret);
    if (ret < 0) {
        LOGI("jni init fail!\n");
        return false;
    }
    ss_mgr->set_imgproc_with_opencv(true);

    return true;
}

/**
 *  description: Input the frame data to the model, and the model will output
 *    the inference result, which includes the action, back and head poses..
 *
 *  @param buf: the single frame's data.
 *  @param width: the input frame's width.
 *  @param height: the input frame's height.
 *
 *  @return: the study status model's result.
 */
extern "C" JNIEXPORT jintArray JNICALL
Java_com_youdao_ocr_StudyStatus_getStudyStatusYUV(
        JNIEnv* env, jobject /* this */,
        jbyteArray buf, jint width, jint height) {
    jbyte *data  = (jbyte *) env->GetByteArrayElements(buf, 0);
    cv::Mat mat = cv::Mat(height + height / 2, width, CV_8UC1, (unsigned char *)data);

    StudyStatusResult ss_res = ss_mgr->get_study_status(mat, width, height);

    jintArray arr_res = nullptr;
    arr_res = env->NewIntArray(CLASS_NUM + 1);
    int *buffer = new int[CLASS_NUM + 1];
    buffer[0] = static_cast<int>(ss_res.back);
    buffer[1] = static_cast<int>(ss_res.head);
    buffer[2] = static_cast<int>(ss_res.action);
    if (ss_mgr->get_shading()) {
        buffer[3] = 1;
    } else {
        buffer[3] = 0;
    }
    LOGI("current study status result: [BackPose: %d, HeadPose: %d, ActionPose: %d, \
            Shading: %d]\n", buffer[0], buffer[1], buffer[2], buffer[3]);

    env->SetIntArrayRegion(arr_res, 0, CLASS_NUM + 1, buffer);
    delete [] buffer;

    env->ReleaseByteArrayElements(buf, data, 0);
    return arr_res;
}

extern "C" JNIEXPORT jintArray JNICALL
Java_com_youdao_ocr_AladdinStudyStatus_getStudyStatusYUV(
        JNIEnv* env, jobject /* this */,
        jbyteArray buf, jint width, jint height) {
    jbyte *data  = (jbyte *) env->GetByteArrayElements(buf, 0);
    cv::Mat mat = cv::Mat(height + height / 2, width, CV_8UC1, (unsigned char *)data);

    StudyStatusResult ss_res = ss_mgr->get_study_status(mat, width, height);

    jintArray arr_res = nullptr;
    arr_res = env->NewIntArray(CLASS_NUM + 1);
    int *buffer = new int[CLASS_NUM + 1];
    buffer[0] = static_cast<int>(ss_res.back);
    buffer[1] = static_cast<int>(ss_res.head);
    buffer[2] = static_cast<int>(ss_res.action);
    if (ss_mgr->get_shading()) {
        buffer[3] = 1;
    } else {
        buffer[3] = 0;
    }
    LOGI("current study status result: [BackPose: %d, HeadPose: %d, ActionPose: %d, \
            Shading: %d]\n", buffer[0], buffer[1], buffer[2], buffer[3]);

    env->SetIntArrayRegion(arr_res, 0, CLASS_NUM + 1, buffer);
    delete [] buffer;

    env->ReleaseByteArrayElements(buf, data, 0);
    return arr_res;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_youdao_ocr_StudyStatus_getSmartLightResultYUV(
        JNIEnv* env, jobject /* this */,
        jbyteArray buf, jint width, jint height) {
    jbyte *data  = (jbyte *) env->GetByteArrayElements(buf, 0);
    cv::Mat mat = cv::Mat(height + height / 2, width, CV_8UC1, (unsigned char *)data);

    SmartLightResult sl_res = ss_mgr->get_smart_light(mat, width, height);

    jint res = static_cast<int>(sl_res);
    LOGI("current smart light result: %d.\n", res);

    env->ReleaseByteArrayElements(buf, data, 0);
    return res;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_youdao_ocr_AladdinStudyStatus_getSmartLightResultYUV(
        JNIEnv* env, jobject /* this */,
        jbyteArray buf, jint width, jint height) {
    jbyte *data  = (jbyte *) env->GetByteArrayElements(buf, 0);
    cv::Mat mat = cv::Mat(height + height / 2, width, CV_8UC1, (unsigned char *)data);

    SmartLightResult sl_res = ss_mgr->get_smart_light(mat, width, height);

    jint res = static_cast<int>(sl_res);
    LOGI("current smart light result: %d.\n", res);

    env->ReleaseByteArrayElements(buf, data, 0);
    return res;
}

extern "C" JNIEXPORT jintArray JNICALL
Java_com_youdao_ocr_StudyStatus_getCurrStudyStatusResult(
        JNIEnv* env, jobject /* this */) {
    StudyStatusResult ss_res = ss_mgr->get_curr_study_status_result();

    jintArray arr_res = nullptr;
    arr_res = env->NewIntArray(CLASS_NUM + 1);
    int *buffer = new int[CLASS_NUM + 1];
    buffer[0] = static_cast<int>(ss_res.back);
    buffer[1] = static_cast<int>(ss_res.head);
    buffer[2] = static_cast<int>(ss_res.action);
    LOGI("current study status result: [BackPose: %d, HeadPose: %d, ActionPose: %d]\n", \
            buffer[0], buffer[1], buffer[2], buffer[3]);

    if (ss_mgr->get_shading()) {
        buffer[3] = 1;
    } else {
        buffer[3] = 0;
    }

    env->SetIntArrayRegion(arr_res, 0, CLASS_NUM + 1, buffer);
    delete buffer;

    return arr_res;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_youdao_ocr_StudyStatus_isBodyFront(
        JNIEnv* env, jobject /* this */) {
    StudyStatusResult ss_res = ss_mgr->get_curr_study_status_result();

    bool is_body_front = true;
    if (ss_res.action == ActionPose::nobody) {
        is_body_front = false;
    }
    
    return is_body_front;
}

extern "C" JNIEXPORT jintArray JNICALL
Java_com_youdao_ocr_AladdinStudyStatus_getCurrStudyStatusResult(
        JNIEnv* env, jobject /* this */) {
    StudyStatusResult ss_res = ss_mgr->get_curr_study_status_result();

    jintArray arr_res = nullptr;
    arr_res = env->NewIntArray(CLASS_NUM + 1);
    int *buffer = new int[CLASS_NUM + 1];
    buffer[0] = static_cast<int>(ss_res.back);
    buffer[1] = static_cast<int>(ss_res.head);
    buffer[2] = static_cast<int>(ss_res.action);
    LOGI("current study status result: [BackPose: %d, HeadPose: %d, ActionPose: %d]\n", \
            buffer[0], buffer[1], buffer[2], buffer[3]);

    if (ss_mgr->get_shading()) {
        buffer[3] = 1;
    } else {
        buffer[3] = 0;
    }

    env->SetIntArrayRegion(arr_res, 0, CLASS_NUM + 1, buffer);
    delete buffer;

    return arr_res;
}

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_youdao_ocr_StudyStatus_getShadingScore(
        JNIEnv* env, jobject /* this */) {
    float mean = ss_mgr->get_shading_mean();
    float stddev = ss_mgr->get_shading_stddev();

    jfloatArray arr_res = nullptr;
    arr_res = env->NewFloatArray(2);
    float *buffer = new float[2];
    buffer[0] = mean;
    buffer[1] = stddev;

    env->SetFloatArrayRegion(arr_res, 0, 2, buffer);
    delete buffer;

    return arr_res;
}

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_youdao_ocr_AladdinStudyStatus_getShadingScore(
        JNIEnv* env, jobject /* this */) {
    float mean = ss_mgr->get_shading_mean();
    float stddev = ss_mgr->get_shading_stddev();

    jfloatArray arr_res = nullptr;
    arr_res = env->NewFloatArray(2);
    float *buffer = new float[2];
    buffer[0] = mean;
    buffer[1] = stddev;

    env->SetFloatArrayRegion(arr_res, 0, 2, buffer);
    delete buffer;

    return arr_res;
}

/**
 *  description: Release the memory resource of tipocr model.
 *
 *  @return: true for releasing successfully, while failure with false.
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_com_youdao_ocr_StudyStatus_release(
        JNIEnv* env,
        jobject /* this */) {
    std::lock_guard<std::mutex> guard(g_mtx);

    if (ss_mgr != nullptr) {
        delete ss_mgr;
        ss_mgr = nullptr;

        if (src_i420_data != nullptr) {
            free(src_i420_data);
        }

        return true;
    }

    // we should never come here!
    LOGI("ERROR! Release the null pointer!");
    return false;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_youdao_ocr_AladdinStudyStatus_release(
        JNIEnv* env,
        jobject /* this */) {
    LOGI("study_status_start_to_release 0 !");
    std::lock_guard<std::mutex> guard(g_mtx);
    LOGI("study_status_start_to_release 1!");

    if (ss_mgr != nullptr) {
        LOGI("release_0");
        delete ss_mgr;
        LOGI("release_1");
        ss_mgr = nullptr;

        if (src_i420_data != nullptr) {
            free(src_i420_data);
        }

        return true;
    }
    LOGI("study_status_start_to_release 2!");

    // we should never come here!
    LOGI("ERROR! Release the null pointer!");
    return false;
}
