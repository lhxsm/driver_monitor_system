#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <csignal>
#include "../include/driver_monitor.hpp"
#include "../include/config_reader.hpp"
#include "../include/event_logger.hpp"

// 全局变量，用于信号处理
std::atomic<bool> g_running(true);

// 信号处理函数
void signalHandler(int signum) {
    std::cout << "接收到信号 " << signum << "，准备退出程序" << std::endl;
    g_running = false;
}

// 行为检测回调函数
void behaviorCallback(std::shared_ptr<EventLogger> logger, DriverBehavior behavior, const std::string& message, const cv::Mat& frame) {
    // 记录事件
    if (behavior != DriverBehavior::NORMAL) {
        logger->logEvent(behavior, message, frame);
    }
    
    // 在控制台输出检测结果
    std::cout << "检测到行为: " << DriverMonitor::behaviorToString(behavior) << std::endl;
    std::cout << "提示信息: " << message << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        // 检查命令行参数
        std::string config_file = "config/config.json";
        if (argc > 1) {
            config_file = argv[1];
        }
        
        std::cout << "使用配置文件: " << config_file << std::endl;
        
        // 注册信号处理函数
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
        
        // 加载配置
        std::shared_ptr<ConfigReader> config = std::make_shared<ConfigReader>(config_file);
        
        // 创建事件记录器
        std::shared_ptr<EventLogger> logger = std::make_shared<EventLogger>(
            config->getEventsDir(),
            config->getImagesDir()
        );
        logger->setSaveImages(config->isSaveImages());
        
        // 创建驾驶行为监测系统
        std::shared_ptr<DriverMonitor> monitor = std::make_shared<DriverMonitor>();
        
        // 初始化摄像头
        if (!monitor->initialize(config->getCameraDeviceId())) {
            std::cerr << "初始化驾驶行为监测系统失败" << std::endl;
            return 1;
        }
        
        // 创建窗口用于显示视频流
        cv::namedWindow("驾驶行为监测系统", cv::WINDOW_AUTOSIZE);
        
        // 启动驾驶行为监测
        monitor->start(std::bind(behaviorCallback, logger, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        
        std::cout << "驾驶行为监测系统已启动" << std::endl;
        std::cout << "可以检测的行为: 闭眼、打哈欠、喝水、打电话" << std::endl;
        std::cout << "按 'q' 键或 Ctrl+C 退出程序" << std::endl;
        
        // 主循环
        char key = 0;
        while (g_running && key != 'q' && key != 'Q') {
            // 获取当前帧
            cv::Mat frame = monitor->getCurrentFrame();
            
            if (!frame.empty()) {
                // 获取当前行为
                DriverBehavior behavior = monitor->getCurrentBehavior();
                std::string behavior_str = DriverMonitor::behaviorToString(behavior);
                std::string message = DriverMonitor::getBehaviorMessage(behavior);
                
                // 在图像上显示行为和提示信息
                cv::putText(frame, "行为: " + behavior_str, cv::Point(10, 30), 
                           cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
                cv::putText(frame, "提示: " + message, cv::Point(10, 60), 
                           cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
                
                // 显示图像
                cv::imshow("驾驶行为监测系统", frame);
            }
            
            // 等待按键
            key = cv::waitKey(30);
        }
        
        // 停止驾驶行为监测
        monitor->stop();
        
        // 关闭窗口
        cv::destroyAllWindows();
        
        std::cout << "驾驶行为监测系统已退出" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "发生异常: " << e.what() << std::endl;
        return 1;
    }
}
