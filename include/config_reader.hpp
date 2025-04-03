#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

// 使用nlohmann/json库
using json = nlohmann::json;

class ConfigReader {
public:
    ConfigReader(const std::string& config_file = "config/config.json");
    ~ConfigReader() = default;

    // 获取摄像头设备ID
    int getCameraDeviceId() const;
    
    // 获取摄像头宽度
    int getCameraWidth() const;
    
    // 获取摄像头高度
    int getCameraHeight() const;
    
    // 获取摄像头帧率
    int getCameraFps() const;
    
    // 获取眼睛纵横比阈值
    double getEARThreshold() const;
    
    // 获取嘴部纵横比阈值
    double getMARThreshold() const;
    
    // 获取闭眼帧数阈值
    int getEyeClosedFrames() const;
    
    // 获取哈欠帧数阈值
    int getYawningFrames() const;
    
    // 获取喝水帧数阈值
    int getDrinkingFrames() const;
    
    // 获取打电话帧数阈值
    int getPhoneCallingFrames() const;
    
    // 是否启用声音警报
    bool isEnableSound() const;
    
    // 获取声音音量
    int getSoundVolume() const;
    
    // 是否显示警告
    bool isDisplayWarning() const;
    
    // 是否记录事件
    bool isLogEvents() const;
    
    // 获取面部特征点模型路径
    std::string getFaceLandmarkModel() const;
    
    // 是否保存事件
    bool isSaveEvents() const;
    
    // 获取事件目录
    std::string getEventsDir() const;
    
    // 是否保存图像
    bool isSaveImages() const;
    
    // 获取图像目录
    std::string getImagesDir() const;
    
    // 重新加载配置文件
    bool reload();
    
    // 获取配置文件路径
    std::string getConfigFilePath() const;

private:
    // 加载配置文件
    bool loadConfig();
    
    // 配置文件路径
    std::string _configFile;
    
    // 配置数据
    json _config;
};
