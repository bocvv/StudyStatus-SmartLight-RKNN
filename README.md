# 学情分析(Study Status) & 聪明的光(Smart Light) SDK
## 简介
该sdk提供了对离线学情分析接口的jni相关封装，包含初始化、逐帧推理和释放资源等三个接口，另外在data文件夹中包含了要用到的模型、配置文件。
另外也提供了对自适应调光接口的jni相关封装，包含初始化（init）、逐帧推理（getSmartLightResultYUV）和释放资源（release）等三个接口, 其中init和release接口与学情的复用。  

## 具体接口
根据需求我们提供了Native C++接口供安卓层直接调用，具体接口格式和返回值如下所示：
- jboolean Java_com_youdao_ocr_StudyStatus_init(JNIEnv* env, jobject, jstring configFilePath, jboolean useSmartLight, jboolean useDVT2):
  + 说明：初始化函数，只需要在程序开始处执行一次.
  + useSmartLight: 控制是否开启“聪明的光功能”，为true则开启，反之关闭.
  + useDVT2: 目前的测试机型选择，为true表示部署机型为dvt2，反之则表示为手版机器.
  + return: 释放成功则返回true，否则返回false.

- jintArray Java_com_youdao_ocr_StudyStatus_getStudyStatusYUV(JNIEnv* env, jobject, jbyteArray buf, jint width, jint height): 
  + buf: 当前帧的数据
  + width: 当前帧的宽
  + height: 当前帧的高
  + return: jintArray类型

- jint Java_com_youdao_ocr_StudyStatus_getSmartLightResultYUV(JNIEnv* env, jobject, jbyteArray buf, jint width, jint height):
  + 说明：输入当前帧的数据更新并获取最新聪明的光状态值。
  + buf: 当前帧的数据
  + width: 当前帧的宽
  + height: 当前帧的高
  + return: 当前自适应光的计算结果，具体数值对应字段如下：
    - 0: textbook（教材可能的学习光）
    - 1: cartoon（创作绘本绘画的创作光光）
    - 2: online_class（平板、电脑的网课光）
    - 3: relax（玩手机、玩具、吃喝、睡觉的放松光）
    - 4: auto_light（学情结果不确定是哪一种明确场景，设为自动调亮度）
    - shading: 当前摄像头被物体遮挡.

- jboolean Java_com_youdao_ocr_StudyStatus_release(JNIEnv* env, jobject):   
  + 说明：释放模块占用的所有资源，只须在程序结束位置执行一次.
  + return：释放成功则返回true，否则返回false.

- jboolean Java_com_youdao_ocr_StudyStatus_isBodyFront(JNIEnv* env, jobject):   
  + 说明：辅助判断台灯前是否有人.
  + return：若有人则返回true，无人则返回false.

## 快速开始
- 将文件夹中focus_mode文件夹放到台灯指定路径/etc/ 下面；
- 在安卓中加载jni动态库，调用对应jni接口.

## 注意事项
### 配置文件
在使用前需要修改配置文件，配置文件存放位置为data/cfg/tipdict.cfg, 其中有若干需要配置的项，这里需要强调的有下面几个参数：
- data_dir: data文件夹的prefix，末尾不要加'/', 该文件夹存放了我们要用到的一些模型文件以及配置文件等.
- back_threshold: 设定姿态和学情分析的阈值，默认是0.51，值越低，检测标准就越严格, 一般不需要改动.
- head_threshold: 默认是0.4，值越低，检测标准就越严格, 一般不需要改动.
- action_threshold: 默认是0.25，值越低，检测标准就越严格, 一般不需要改动.

### 模型文件
自适应调光使用的模型文件与学情分析是同一份，目前默认存放位置为/etc/focus_mode/data/路径下，一般不用修改。  
需要注意的是目前模型不是最终版本的模型，可能在精度上有一定偏差，属于正常现象。

### native C++所需动态库
存放路径为libs/arm64-v8a.
