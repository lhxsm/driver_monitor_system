#include "../include/config_reader.hpp"
#include <iostream>
#include <fstream>

ConfigReader::ConfigReader(const std::string& config_file)
    : _configFile(config_file) {
    loadConfig();
}

bool ConfigReader::loadConfig() {
    try {
        // 打开配置文件
        std::ifstream file(_configFile);
        if (!file.is_open()) {
            std::cerr << "无法打开配置文件: " << _configFile << std::endl;
            return false;
        }
        
        // 解析JSON
        file >> _config;
        file.close();
        
        std::cout << "成功加载配置文件: " << _configFile << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载配置文件失败: " << e.what() << std::endl;
        return false;
    }
}

bool ConfigReader::reload() {
    return loadConfig();
}

std::string ConfigReader::getConfigFilePath() const {
    return _configFile;
}

int ConfigReader::getCameraDeviceId() const {
    try {
        return _config["camera"]["device_id"];
    } catch (const std::exception& e) {
        std::cerr << "获取摄像头设备ID失败: " << e.what() << std::endl;
        return 0; // 默认值
    }
}

int ConfigReader::getCameraWidth() const {
    try {
        return _config["camera"]["width"];
    } catch (const std::exception& e) {
        std::cerr << "获取摄像头宽度失败: " << e.what() << std::endl;
        return 640; // 默认值
    }
}

int ConfigReader::getCameraHeight() const {
    try {
        return _config["camera"]["height"];
    } catch (const std::exception& e) {
        std::cerr << "获取摄像头高度失败: " << e.what() << std::endl;
        return 480; // 默认值
    }
}

int ConfigReader::getCameraFps() const {
    try {
        return _config["camera"]["fps"];
    } catch (const std::exception& e) {
        std::cerr << "获取摄像头帧率失败: " << e.what() << std::endl;
        return 30; // 默认值
    }
}

double ConfigReader::getEARThreshold() const {
    try {
        return _config["detection"]["ear_threshold"];
    } catch (const std::exception& e) {
        std::cerr << "获取眼睛纵横比阈值失败: " << e.what() << std::endl;
        return 0.25; // 默认值
    }
}

double ConfigReader::getMARThreshold() const {
    try {
        return _config["detection"]["mar_threshold"];
    } catch (const std::exception& e) {
        std::cerr << "获取嘴部纵横比阈值失败: " << e.what() << std::endl;
        return 0.6; // 默认值
    }
}

int ConfigReader::getEyeClosedFrames() const {
    try {
        return _config["detection"]["eye_closed_frames"];
    } catch (const std::exception& e) {
        std::cerr << "获取闭眼帧数阈值失败: " << e.what() << std::endl;
        return 3; // 默认值
    }
}

int ConfigReader::getYawningFrames() const {
    try {
        return _config["detection"]["yawning_frames"];
    } catch (const std::exception& e) {
        std::cerr << "获取哈欠帧数阈值失败: " << e.what() << std::endl;
        return 5; // 默认值
    }
}

int ConfigReader::getDrinkingFrames() const {
    try {
        return _config["detection"]["drinking_frames"];
    } catch (const std::exception& e) {
        std::cerr << "获取喝水帧数阈值失败: " << e.what() << std::endl;
        return 3; // 默认值
    }
}

int ConfigReader::getPhoneCallingFrames() const {
    try {
        return _config["detection"]["phone_calling_frames"];
    } catch (const std::exception& e) {
        std::cerr << "获取打电话帧数阈值失败: " << e.what() << std::endl;
        return 5; // 默认值
    }
}

bool ConfigReader::isEnableSound() const {
    try {
        return _config["alert"]["enable_sound"];
    } catch (const std::exception& e) {
        std::cerr << "获取是否启用声音警报失败: " << e.what() << std::endl;
        return true; // 默认值
    }
}

int ConfigReader::getSoundVolume() const {
    try {
        return _config["alert"]["sound_volume"];
    } catch (const std::exception& e) {
        std::cerr << "获取声音音量失败: " << e.what() << std::endl;
        return 80; // 默认值
    }
}

bool ConfigReader::isDisplayWarning() const {
    try {
        return _config["alert"]["display_warning"];
    } catch (const std::exception& e) {
        std::cerr << "获取是否显示警告失败: " << e.what() << std::endl;
        return true; // 默认值
    }
}

bool ConfigReader::isLogEvents() const {
    try {
        return _config["alert"]["log_events"];
    } catch (const std::exception& e) {
        std::cerr << "获取是否记录事件失败: " << e.what() << std::endl;
        return true; // 默认值
    }
}

std::string ConfigReader::getFaceLandmarkModel() const {
    try {
        return _config["model"]["face_landmark_model"];
    } catch (const std::exception& e) {
        std::cerr << "获取面部特征点模型路径失败: " << e.what() << std::endl;
        return "shape_predictor_68_face_landmarks.dat"; // 默认值
    }
}

bool ConfigReader::isSaveEvents() const {
    try {
        return _config["output"]["save_events"];
    } catch (const std::exception& e) {
        std::cerr << "获取是否保存事件失败: " << e.what() << std::endl;
        return true; // 默认值
    }
}

std::string ConfigReader::getEventsDir() const {
    try {
        return _config["output"]["events_dir"];
    } catch (const std::exception& e) {
        std::cerr << "获取事件目录失败: " << e.what() << std::endl;
        return "events"; // 默认值
    }
}

bool ConfigReader::isSaveImages() const {
    try {
        return _config["output"]["save_images"];
    } catch (const std::exception& e) {
        std::cerr << "获取是否保存图像失败: " << e.what() << std::endl;
        return true; // 默认值
    }
}

std::string ConfigReader::getImagesDir() const {
    try {
        return _config["output"]["images_dir"];
    } catch (const std::exception& e) {
        std::cerr << "获取图像目录失败: " << e.what() << std::endl;
        return "images"; // 默认值
    }
}
