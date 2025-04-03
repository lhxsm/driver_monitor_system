#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "driver_monitor.hpp"

// 事件记录结构体
struct BehaviorEvent {
    DriverBehavior behavior;    // 行为类型
    std::string message;        // 提示信息
    std::string timestamp;      // 时间戳
    std::string image_path;     // 图像路径（如果有）
};

class EventLogger {
public:
    EventLogger(const std::string& events_dir = "events", const std::string& images_dir = "images");
    ~EventLogger();

    // 记录事件
    bool logEvent(DriverBehavior behavior, const std::string& message, const cv::Mat& image);
    
    // 获取所有记录的事件
    std::vector<BehaviorEvent> getEvents() const;
    
    // 清除所有事件记录
    void clearEvents();
    
    // 设置事件目录
    void setEventsDir(const std::string& events_dir);
    
    // 设置图像目录
    void setImagesDir(const std::string& images_dir);
    
    // 是否保存图像
    void setSaveImages(bool save_images);
    
    // 获取事件目录
    std::string getEventsDir() const;
    
    // 获取图像目录
    std::string getImagesDir() const;
    
    // 是否保存图像
    bool isSaveImages() const;

private:
    // 获取当前时间戳
    std::string getCurrentTimestamp() const;
    
    // 确保目录存在
    bool ensureDirectoryExists(const std::string& dir) const;
    
    // 保存图像
    std::string saveImage(const cv::Mat& image, const std::string& prefix) const;

private:
    std::string _eventsDir;         // 事件目录
    std::string _imagesDir;         // 图像目录
    bool _saveImages;               // 是否保存图像
    
    std::vector<BehaviorEvent> _events;  // 事件记录
    mutable std::mutex _eventsMutex;     // 事件记录互斥锁
    
    std::ofstream _logFile;         // 日志文件
};
