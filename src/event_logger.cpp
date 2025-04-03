#include "../include/event_logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

// 使用C++17的文件系统库
namespace fs = std::filesystem;

EventLogger::EventLogger(const std::string& events_dir, const std::string& images_dir)
    : _eventsDir(events_dir),
      _imagesDir(images_dir),
      _saveImages(true) {
    
    // 确保目录存在
    ensureDirectoryExists(_eventsDir);
    ensureDirectoryExists(_imagesDir);
    
    // 打开日志文件
    std::string log_file_path = _eventsDir + "/events.log";
    _logFile.open(log_file_path, std::ios::app);
    
    if (!_logFile.is_open()) {
        std::cerr << "无法打开日志文件: " << log_file_path << std::endl;
    } else {
        std::cout << "事件日志将保存到: " << log_file_path << std::endl;
    }
}

EventLogger::~EventLogger() {
    if (_logFile.is_open()) {
        _logFile.close();
    }
}

bool EventLogger::logEvent(DriverBehavior behavior, const std::string& message, const cv::Mat& image) {
    try {
        // 获取当前时间戳
        std::string timestamp = getCurrentTimestamp();
        
        // 保存图像（如果启用）
        std::string image_path;
        if (_saveImages && !image.empty()) {
            std::string prefix = DriverMonitor::behaviorToString(behavior);
            image_path = saveImage(image, prefix);
        }
        
        // 创建事件记录
        BehaviorEvent event;
        event.behavior = behavior;
        event.message = message;
        event.timestamp = timestamp;
        event.image_path = image_path;
        
        // 添加到事件列表
        {
            std::lock_guard<std::mutex> lock(_eventsMutex);
            _events.push_back(event);
        }
        
        // 写入日志文件
        if (_logFile.is_open()) {
            _logFile << timestamp << " | " 
                    << DriverMonitor::behaviorToString(behavior) << " | " 
                    << message << " | " 
                    << image_path << std::endl;
            _logFile.flush();
        }
        
        std::cout << "记录事件: " << DriverMonitor::behaviorToString(behavior) 
                 << " 时间: " << timestamp << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "记录事件失败: " << e.what() << std::endl;
        return false;
    }
}

std::vector<BehaviorEvent> EventLogger::getEvents() const {
    std::lock_guard<std::mutex> lock(_eventsMutex);
    return _events;
}

void EventLogger::clearEvents() {
    std::lock_guard<std::mutex> lock(_eventsMutex);
    _events.clear();
}

void EventLogger::setEventsDir(const std::string& events_dir) {
    if (_eventsDir != events_dir) {
        _eventsDir = events_dir;
        ensureDirectoryExists(_eventsDir);
        
        // 重新打开日志文件
        if (_logFile.is_open()) {
            _logFile.close();
        }
        
        std::string log_file_path = _eventsDir + "/events.log";
        _logFile.open(log_file_path, std::ios::app);
    }
}

void EventLogger::setImagesDir(const std::string& images_dir) {
    if (_imagesDir != images_dir) {
        _imagesDir = images_dir;
        ensureDirectoryExists(_imagesDir);
    }
}

void EventLogger::setSaveImages(bool save_images) {
    _saveImages = save_images;
}

std::string EventLogger::getEventsDir() const {
    return _eventsDir;
}

std::string EventLogger::getImagesDir() const {
    return _imagesDir;
}

bool EventLogger::isSaveImages() const {
    return _saveImages;
}

std::string EventLogger::getCurrentTimestamp() const {
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    // 获取毫秒部分
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    // 格式化时间戳
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

bool EventLogger::ensureDirectoryExists(const std::string& dir) const {
    try {
        if (!fs::exists(dir)) {
            fs::create_directories(dir);
            std::cout << "创建目录: " << dir << std::endl;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "创建目录失败: " << dir << " - " << e.what() << std::endl;
        return false;
    }
}

std::string EventLogger::saveImage(const cv::Mat& image, const std::string& prefix) const {
    try {
        // 确保目录存在
        ensureDirectoryExists(_imagesDir);
        
        // 生成文件名
        std::string timestamp = getCurrentTimestamp();
        std::string filename = prefix + "_" + timestamp + ".jpg";
        // 替换文件名中的非法字符
        std::replace(filename.begin(), filename.end(), ' ', '_');
        std::replace(filename.begin(), filename.end(), ':', '-');
        
        // 完整路径
        std::string filepath = _imagesDir + "/" + filename;
        
        // 保存图像
        cv::imwrite(filepath, image);
        
        std::cout << "保存图像: " << filepath << std::endl;
        
        return filepath;
    } catch (const std::exception& e) {
        std::cerr << "保存图像失败: " << e.what() << std::endl;
        return "";
    }
}
