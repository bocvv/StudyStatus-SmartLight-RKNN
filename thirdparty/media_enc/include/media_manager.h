#ifndef _MEDIA_MANAGER_H
#define _MEDIA_MANAGER_H

#include <stdint.h>
#include <string>

typedef enum {
    M_SUCCESS                 = 0,
    M_OK                      = 0,
    M_EXIT                    = 1000,

    M_NOK                     = -1,
    M_ERR_UNKNOW              = -2,
    M_ERR_NULL_PTR            = -3,
    M_ERR_MALLOC              = -4,
    M_ERR_OPEN_FILE           = -5,
    M_ERR_VALUE               = -6,
    M_ERR_READ_BIT            = -7,
    M_ERR_TIMEOUT             = -8,
    M_ERR_PERM                = -9,

    M_ERR_BASE                = -1000,

    /* The error in stream processing */
    M_ERR_LIST_STREAM         = M_ERR_BASE - 1,
    M_ERR_INIT                = M_ERR_BASE - 2,
    M_ERR_VPU_CODEC_INIT      = M_ERR_BASE - 3,
    M_ERR_STREAM              = M_ERR_BASE - 4,
    M_ERR_FATAL_THREAD        = M_ERR_BASE - 5,
    M_ERR_NOMEM               = M_ERR_BASE - 6,
    M_ERR_PROTOL              = M_ERR_BASE - 7,
    M_FAIL_SPLIT_FRAME        = M_ERR_BASE - 8,
    M_ERR_VPUHW               = M_ERR_BASE - 9,
    M_EOS_STREAM_REACHED      = M_ERR_BASE - 11,
    M_ERR_BUFFER_FULL         = M_ERR_BASE - 12,
    M_ERR_DISPLAY_FULL        = M_ERR_BASE - 13,
    M_ERR_LOG_PUSH            = M_ERR_BASE - 14,
} M_RET;

/* mpp color format index definition */
#define FRAME_FMT_YUV           (0x00000000)
#define FRAME_FMT_RGB           (0x00010000)
typedef enum {
    FMT_YUV420SP        = (FRAME_FMT_YUV + 0),  /* YYYY... UV... (NV12)     */
    /*
     * A rockchip specific pixel format, without gap between pixel aganist
     * the P010_10LE/P010_10BE
     */
    FMT_YUV420SP_10BIT  = (FRAME_FMT_YUV + 1),
    FMT_YUV422SP        = (FRAME_FMT_YUV + 2),  /* YYYY... UVUV... (NV16)   */
    FMT_YUV422SP_10BIT  = (FRAME_FMT_YUV + 3),  ///< Not part of ABI
    FMT_YUV420P         = (FRAME_FMT_YUV + 4),  /* YYYY... U...V...  (I420) */
    FMT_YUV420SP_VU     = (FRAME_FMT_YUV + 5),  /* YYYY... VUVUVU... (NV21) */
    FMT_YUV422P         = (FRAME_FMT_YUV + 6),  /* YYYY... UU...VV...(422P) */
    FMT_YUV422SP_VU     = (FRAME_FMT_YUV + 7),  /* YYYY... VUVUVU... (NV61) */
    FMT_YUV422_YUYV     = (FRAME_FMT_YUV + 8),  /* YUYVYUYV... (YUY2)       */
    FMT_YUV422_YVYU     = (FRAME_FMT_YUV + 9),  /* YVYUYVYU... (YVY2)       */
    FMT_YUV422_UYVY     = (FRAME_FMT_YUV + 10), /* UYVYUYVY... (UYVY)       */
    FMT_YUV422_VYUY     = (FRAME_FMT_YUV + 11), /* VYUYVYUY... (VYUY)       */
    FMT_YUV400          = (FRAME_FMT_YUV + 12), /* YYYY...                  */
    FMT_YUV440SP        = (FRAME_FMT_YUV + 13), /* YYYY... UVUV...          */
    FMT_YUV411SP        = (FRAME_FMT_YUV + 14), /* YYYY... UV...            */
    FMT_YUV444SP        = (FRAME_FMT_YUV + 15), /* YYYY... UVUVUVUV...      */
    FMT_YUV_BUTT,

    FMT_RGB565          = (FRAME_FMT_RGB + 0),  /* 16-bit RGB               */
    FMT_BGR565          = (FRAME_FMT_RGB + 1),  /* 16-bit RGB               */
    FMT_RGB555          = (FRAME_FMT_RGB + 2),  /* 15-bit RGB               */
    FMT_BGR555          = (FRAME_FMT_RGB + 3),  /* 15-bit RGB               */
    FMT_RGB444          = (FRAME_FMT_RGB + 4),  /* 12-bit RGB               */
    FMT_BGR444          = (FRAME_FMT_RGB + 5),  /* 12-bit RGB               */
    FMT_RGB888          = (FRAME_FMT_RGB + 6),  /* 24-bit RGB               */
    FMT_BGR888          = (FRAME_FMT_RGB + 7),  /* 24-bit RGB               */
    FMT_RGB101010       = (FRAME_FMT_RGB + 8),  /* 30-bit RGB               */
    FMT_BGR101010       = (FRAME_FMT_RGB + 9),  /* 30-bit RGB               */
    FMT_ARGB8888        = (FRAME_FMT_RGB + 10), /* 32-bit RGB               */
    FMT_ABGR8888        = (FRAME_FMT_RGB + 11), /* 32-bit RGB               */
    FMT_BGRA8888        = (FRAME_FMT_RGB + 12), /* 32-bit RGB               */
    FMT_RGBA8888        = (FRAME_FMT_RGB + 13), /* 32-bit RGB               */
    FMT_RGB_BUTT,

    FMT_BUTT,
} FrameFormat;

/**
 * @ingroup rk_mpi
 * @brief Enumeration used to define the possible video compression codings.
 *        sync with the omx_video.h
 *
 * @note  This essentially refers to file extensions. If the coding is
 *        being used to specify the ENCODE type, then additional work
 *        must be done to configure the exact flavor of the compression
 *        to be used.  For decode cases where the user application can
 *        not differentiate between MPEG-4 and H.264 bit streams, it is
 *        up to the codec to handle this.
 */
typedef enum {
    VIDEO_CodingUnused,             /**< Value when coding is N/A */
    VIDEO_CodingAutoDetect,         /**< Autodetection of coding type */
    VIDEO_CodingMPEG2,              /**< AKA: H.262 */
    VIDEO_CodingH263,               /**< H.263 */
    VIDEO_CodingMPEG4,              /**< MPEG-4 */
    VIDEO_CodingWMV,                /**< Windows Media Video (WMV1,WMV2,WMV3)*/
    VIDEO_CodingRV,                 /**< all versions of Real Video */
    VIDEO_CodingAVC,                /**< H.264/AVC */
    VIDEO_CodingMJPEG,              /**< Motion JPEG */
    VIDEO_CodingVP8,                /**< VP8 */
    VIDEO_CodingVP9,                /**< VP9 */
    VIDEO_CodingVC1 = 0x01000000,   /**< Windows Media Video (WMV1,WMV2,WMV3)*/
    VIDEO_CodingFLV1,               /**< Sorenson H.263 */
    VIDEO_CodingDIVX3,              /**< DIVX3 */
    VIDEO_CodingVP6,
    VIDEO_CodingHEVC,               /**< H.265/HEVC */
    VIDEO_CodingAVSPLUS,            /**< AVS+ */
    VIDEO_CodingAVS,                /**< AVS profile=0x20 */
    VIDEO_CodingKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */
    VIDEO_CodingVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    VIDEO_CodingMax = 0x7FFFFFFF
} CodingType;


class MppEncoder;
class UDPSocket;

class MediaManager {
public:
    MediaManager() {}
    MediaManager(const char* module_name, const char* module_version, \
        const char* sdk_name, const char* sdk_version);
    ~MediaManager();

    // 每次编码都需要执行一次该初始化函数, 也就是说每次编码都是独立的
    M_RET init(int width, int height, FrameFormat format, CodingType type, int num_frames, int fps = 30);

    //执行对单次图像数据的编码
    M_RET run(void *data);

    // 手动终止当前编码过程
    void stop();

    // 判断当前是否正在进行编码
    bool is_running();

    // 判断当前所有推理线程是否已结束
    bool has_finished();

    // 执行close会清除涉及本次编码的所有数据，包括编码后结果的缓存
    M_RET close();

    // 对于单帧图像的一步式编码接口，只需执行该函数即可得到一张图像的jpeg编码结果
    M_RET enc_image_jpg(void *data, int width, int height, FrameFormat format);

    // 获取编码结果的缓存指针, 由于close会清除缓存结果，所以应在close前执行
    uint8_t *get_encode_buffer();

    // 获取编码结果的总长度, 单位为Byte
    int get_total_len();

    bool log_push(int log_type , const char* log_buf, int log_len, \
            long log_time, const char* log_str);

    bool send_log(int log_type/* log_type取值为0-4，分别对应IMAGE/VIDEO/AUDIO/TEXTS/OTHER */);

private:
    MppEncoder *mpp_enc{nullptr};

    // 日志模块相关变量
    void *log_handle{0};

    // 日志模块相关函数
    bool log_init(const char* module_name, const char* module_version, \
            const char* sdk_name, const char* sdk_version);
    std::string log_vers();
    bool log_free();
};

#endif /* !_MEDIA_MANAGER_H */
