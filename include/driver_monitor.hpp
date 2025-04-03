#pragma once

#include <string>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

// 驾驶行为类型
enum class DriverBehavior {
    NORMAL,          // 正常驾驶
    EYES_CLOSED,     // 闭眼
    YAWNING,         // 打哈欠
    DRINKING,        // 喝水
    PHONE_CALLING,   // 打电话
    UNKNOWN          // 未知行为
};

// 行为检测结果回调函数类型
using BehaviorCallback = std::function<void(DriverBehavior, const std::string&, const cv::Mat&)>;

class DriverMonitor {
public:
    DriverMonitor();
    ~DriverMonitor();

    // 初始化摄像头和模型
    bool initialize(int camera_id = 0);
    
    // 启动监测
    bool start(BehaviorCallback callback);
    
    // 停止监测
    void stop();
    
    // 获取当前帧
    cv::Mat getCurrentFrame() const;
    
    // 获取当前检测到的行为
    DriverBehavior getCurrentBehavior() const;
    
    // 行为类型转字符串
    static std::string behaviorToString(DriverBehavior behavior);
    
    // 获取行为提示信息
    static std::string getBehaviorMessage(DriverBehavior behavior);

private:
    // 监测线程函数
    void monitorThread();
    
    // 检测眼睛状态 (闭眼/睁眼)
    bool detectEyesClosed(const dlib::full_object_detection& shape);
    
    // 检测哈欠
    bool detectYawning(const dlib::full_object_detection& shape);
    
    // 检测喝水
    bool detectDrinking(const cv::Mat& frame);
    
    // 检测打电话
    bool detectPhoneCalling(const cv::Mat& frame, const dlib::full_object_detection& shape);
    
    // 计算眼睛纵横比 (Eye Aspect Ratio)
    double calculateEAR(const std::vector<dlib::point>& eye);
    
    // 计算嘴部纵横比 (Mouth Aspect Ratio)
    double calculateMAR(const dlib::full_object_detection& shape);

private:
    // OpenCV相关
    cv::VideoCapture _camera;
    cv::Mat _currentFrame;
    
    // dlib相关
    dlib::frontal_face_detector _faceDetector;
    dlib::shape_predictor _shapePredictor;
    
    // 线程相关
    std::thread _monitorThread;
    std::atomic<bool> _running;
    
    // 当前检测到的行为
    DriverBehavior _currentBehavior;
    mutable std::mutex _frameMutex;
    mutable std::mutex _behaviorMutex;
    
    // 回调函数
    BehaviorCallback _callback;
    
    // 检测阈值
    double _EAR_THRESHOLD;      // 眼睛纵横比阈值
    double _MAR_THRESHOLD;      // 嘴部纵横比阈值
    int _EYE_CLOSED_FRAMES;     // 连续闭眼帧数阈值
    int _YAWNING_FRAMES;        // 连续哈欠帧数阈值
    
    // 计数器
    int _eyeClosedCounter;
    int _yawningCounter;
    int _drinkingCounter;
    int _phoneCallingCounter;
};
