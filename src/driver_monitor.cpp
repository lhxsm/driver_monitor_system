#include "../include/driver_monitor.hpp"
#include <iostream>
#include <chrono>
#include <cmath>

DriverMonitor::DriverMonitor() 
    : _running(false), 
      _currentBehavior(DriverBehavior::NORMAL),
      _EAR_THRESHOLD(0.25),
      _MAR_THRESHOLD(0.6),
      _EYE_CLOSED_FRAMES(3),
      _YAWNING_FRAMES(5),
      _eyeClosedCounter(0),
      _yawningCounter(0),
      _drinkingCounter(0),
      _phoneCallingCounter(0) {
}

DriverMonitor::~DriverMonitor() {
    stop();
}

bool DriverMonitor::initialize(int camera_id) {
    try {
        // 初始化摄像头
        _camera.open(camera_id);
        if (!_camera.isOpened()) {
            std::cerr << "无法打开摄像头 ID: " << camera_id << std::endl;
            return false;
        }
        
        // 设置摄像头参数
        _camera.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        _camera.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        
        // 初始化dlib人脸检测器
        _faceDetector = dlib::get_frontal_face_detector();
        
        // 加载面部特征点预测模型
        // 注意：需要下载shape_predictor_68_face_landmarks.dat文件
        // 可以从 http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2 下载
        std::string model_path = "shape_predictor_68_face_landmarks.dat";
        try {
            dlib::deserialize(model_path) >> _shapePredictor;
        } catch (const std::exception& e) {
            std::cerr << "无法加载面部特征点预测模型: " << e.what() << std::endl;
            std::cerr << "请确保 " << model_path << " 文件存在" << std::endl;
            return false;
        }
        
        std::cout << "驾驶行为监测系统初始化成功" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "初始化驾驶行为监测系统失败: " << e.what() << std::endl;
        return false;
    }
}

bool DriverMonitor::start(BehaviorCallback callback) {
    if (_running) {
        std::cout << "驾驶行为监测系统已经在运行中" << std::endl;
        return false;
    }
    
    if (!_camera.isOpened()) {
        std::cerr << "摄像头未初始化，无法启动监测" << std::endl;
        return false;
    }
    
    _callback = callback;
    _running = true;
    _monitorThread = std::thread(&DriverMonitor::monitorThread, this);
    
    std::cout << "驾驶行为监测系统已启动" << std::endl;
    return true;
}

void DriverMonitor::stop() {
    if (!_running) {
        return;
    }
    
    _running = false;
    
    if (_monitorThread.joinable()) {
        _monitorThread.join();
    }
    
    // 释放摄像头
    if (_camera.isOpened()) {
        _camera.release();
    }
    
    std::cout << "驾驶行为监测系统已停止" << std::endl;
}

cv::Mat DriverMonitor::getCurrentFrame() const {
    std::lock_guard<std::mutex> lock(_frameMutex);
    return _currentFrame.clone();
}

DriverBehavior DriverMonitor::getCurrentBehavior() const {
    std::lock_guard<std::mutex> lock(_behaviorMutex);
    return _currentBehavior;
}

std::string DriverMonitor::behaviorToString(DriverBehavior behavior) {
    switch (behavior) {
        case DriverBehavior::NORMAL:
            return "正常驾驶";
        case DriverBehavior::EYES_CLOSED:
            return "闭眼";
        case DriverBehavior::YAWNING:
            return "打哈欠";
        case DriverBehavior::DRINKING:
            return "喝水";
        case DriverBehavior::PHONE_CALLING:
            return "打电话";
        default:
            return "未知行为";
    }
}

std::string DriverMonitor::getBehaviorMessage(DriverBehavior behavior) {
    switch (behavior) {
        case DriverBehavior::NORMAL:
            return "驾驶状态良好，请继续保持";
        case DriverBehavior::EYES_CLOSED:
            return "警告：检测到闭眼行为，请保持警觉！";
        case DriverBehavior::YAWNING:
            return "警告：检测到打哈欠，您可能存在疲劳驾驶风险！";
        case DriverBehavior::DRINKING:
            return "警告：检测到喝水行为，请谨慎驾驶！";
        case DriverBehavior::PHONE_CALLING:
            return "警告：检测到打电话行为，这是危险的分心驾驶行为！";
        default:
            return "未知驾驶行为";
    }
}

void DriverMonitor::monitorThread() {
    cv::Mat frame;
    dlib::cv_image<dlib::bgr_pixel> dlib_frame;
    std::vector<dlib::rectangle> faces;
    
    while (_running) {
        // 捕获一帧
        if (!_camera.read(frame)) {
            std::cerr << "无法从摄像头读取帧" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            continue;
        }
        
        // 更新当前帧
        {
            std::lock_guard<std::mutex> lock(_frameMutex);
            _currentFrame = frame.clone();
        }
        
        // 转换为dlib图像格式
        dlib_frame = dlib::cv_image<dlib::bgr_pixel>(frame);
        
        // 检测人脸
        faces = _faceDetector(dlib_frame);
        
        DriverBehavior detectedBehavior = DriverBehavior::NORMAL;
        
        if (!faces.empty()) {
            // 获取第一个人脸的特征点
            dlib::full_object_detection shape = _shapePredictor(dlib_frame, faces[0]);
            
            // 检测各种行为
            bool eyesClosed = detectEyesClosed(shape);
            bool yawning = detectYawning(shape);
            bool drinking = detectDrinking(frame);
            bool phoneCalling = detectPhoneCalling(frame, shape);
            
            // 根据检测结果更新行为状态
            if (phoneCalling) {
                detectedBehavior = DriverBehavior::PHONE_CALLING;
            } else if (drinking) {
                detectedBehavior = DriverBehavior::DRINKING;
            } else if (eyesClosed) {
                detectedBehavior = DriverBehavior::EYES_CLOSED;
            } else if (yawning) {
                detectedBehavior = DriverBehavior::YAWNING;
            }
            
            // 在图像上绘制人脸特征点
            for (unsigned long i = 0; i < shape.num_parts(); ++i) {
                cv::circle(frame, cv::Point(shape.part(i).x(), shape.part(i).y()), 2, cv::Scalar(0, 255, 0), -1);
            }
            
            // 绘制人脸框
            cv::rectangle(frame, 
                         cv::Point(faces[0].left(), faces[0].top()), 
                         cv::Point(faces[0].right(), faces[0].bottom()), 
                         cv::Scalar(0, 255, 0), 2);
        }
        
        // 更新当前行为
        {
            std::lock_guard<std::mutex> lock(_behaviorMutex);
            if (_currentBehavior != detectedBehavior) {
                _currentBehavior = detectedBehavior;
                
                // 调用回调函数
                if (_callback) {
                    _callback(_currentBehavior, getBehaviorMessage(_currentBehavior), frame);
                }
            }
        }
        
        // 在图像上显示当前行为
        cv::putText(frame, 
                   "行为: " + behaviorToString(detectedBehavior), 
                   cv::Point(10, 30), 
                   cv::FONT_HERSHEY_SIMPLEX, 
                   0.7, 
                   cv::Scalar(0, 0, 255), 
                   2);
        
        // 控制帧率
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

bool DriverMonitor::detectEyesClosed(const dlib::full_object_detection& shape) {
    // 左眼特征点索引 (基于68点模型)
    std::vector<dlib::point> leftEye;
    for (int i = 36; i <= 41; ++i) {
        leftEye.push_back(shape.part(i));
    }
    
    // 右眼特征点索引
    std::vector<dlib::point> rightEye;
    for (int i = 42; i <= 47; ++i) {
        rightEye.push_back(shape.part(i));
    }
    
    // 计算左右眼的EAR
    double leftEAR = calculateEAR(leftEye);
    double rightEAR = calculateEAR(rightEye);
    
    // 取平均值
    double avgEAR = (leftEAR + rightEAR) / 2.0;
    
    // 判断是否闭眼
    if (avgEAR < _EAR_THRESHOLD) {
        _eyeClosedCounter++;
    } else {
        _eyeClosedCounter = 0;
    }
    
    // 连续多帧检测到闭眼才判定为闭眼状态
    return _eyeClosedCounter >= _EYE_CLOSED_FRAMES;
}

bool DriverMonitor::detectYawning(const dlib::full_object_detection& shape) {
    // 计算嘴部纵横比
    double mar = calculateMAR(shape);
    
    // 判断是否打哈欠
    if (mar > _MAR_THRESHOLD) {
        _yawningCounter++;
    } else {
        _yawningCounter = 0;
    }
    
    // 连续多帧检测到嘴巴张开才判定为打哈欠
    return _yawningCounter >= _YAWNING_FRAMES;
}

bool DriverMonitor::detectDrinking(const cv::Mat& frame) {
    // 简化实现：基于手部检测和姿势估计
    // 实际应用中应使用更复杂的手部检测和姿势识别算法
    
    // 这里使用简单的颜色检测和运动检测来模拟
    // 在实际应用中，应该使用更高级的方法，如目标检测网络(YOLO, SSD等)
    
    // 简单模拟：随机概率触发，实际应用中替换为真实检测逻辑
    if (rand() % 100 < 5) {  // 5%的概率触发
        _drinkingCounter++;
    } else {
        _drinkingCounter = std::max(0, _drinkingCounter - 1);
    }
    
    return _drinkingCounter > 3;
}

bool DriverMonitor::detectPhoneCalling(const cv::Mat& frame, const dlib::full_object_detection& shape) {
    // 简化实现：基于手部位置和头部姿势
    // 实际应用中应使用更复杂的手部检测和姿势识别算法
    
    // 检测头部倾斜和手部靠近耳朵的姿势
    // 这里使用简单的启发式方法模拟
    
    // 检查头部是否倾斜
    dlib::point leftEye = shape.part(36);
    dlib::point rightEye = shape.part(45);
    double eyeAngle = std::abs(std::atan2(rightEye.y() - leftEye.y(), rightEye.x() - leftEye.x()) * 180.0 / M_PI);
    
    // 简单模拟：随机概率触发，实际应用中替换为真实检测逻辑
    if (eyeAngle > 10 || rand() % 100 < 3) {  // 头部倾斜或3%的概率触发
        _phoneCallingCounter++;
    } else {
        _phoneCallingCounter = std::max(0, _phoneCallingCounter - 1);
    }
    
    return _phoneCallingCounter > 5;
}

double DriverMonitor::calculateEAR(const std::vector<dlib::point>& eye) {
    // 计算眼睛纵横比 (Eye Aspect Ratio)
    // EAR = (||p2-p6|| + ||p3-p5||) / (2 * ||p1-p4||)
    double a = std::sqrt(std::pow(eye[1].x() - eye[5].x(), 2) + std::pow(eye[1].y() - eye[5].y(), 2));
    double b = std::sqrt(std::pow(eye[2].x() - eye[4].x(), 2) + std::pow(eye[2].y() - eye[4].y(), 2));
    double c = std::sqrt(std::pow(eye[0].x() - eye[3].x(), 2) + std::pow(eye[0].y() - eye[3].y(), 2));
    
    // 避免除零错误
    if (c < 0.1) {
        return 0.0;
    }
    
    return (a + b) / (2.0 * c);
}

double DriverMonitor::calculateMAR(const dlib::full_object_detection& shape) {
    // 计算嘴部纵横比 (Mouth Aspect Ratio)
    // 使用嘴部的6个关键点 (48, 54, 51, 57, 62, 66)
    
    // 计算嘴部高度
    double h1 = std::sqrt(std::pow(shape.part(51).x() - shape.part(57).x(), 2) + 
                         std::pow(shape.part(51).y() - shape.part(57).y(), 2));
    double h2 = std::sqrt(std::pow(shape.part(62).x() - shape.part(66).x(), 2) + 
                         std::pow(shape.part(62).y() - shape.part(66).y(), 2));
    
    // 计算嘴部宽度
    double w = std::sqrt(std::pow(shape.part(48).x() - shape.part(54).x(), 2) + 
                        std::pow(shape.part(48).y() - shape.part(54).y(), 2));
    
    // 避免除零错误
    if (w < 0.1) {
        return 0.0;
    }
    
    return (h1 + h2) / (2.0 * w);
}
