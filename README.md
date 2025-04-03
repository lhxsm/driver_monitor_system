# 驾驶行为监测系统 (DMS)

这是一个基于计算机视觉的驾驶行为监测系统，可以通过USB摄像头实时识别驾驶员的闭眼、打哈欠、喝水和打电话等分心驾驶行为，并给出相应的警告提示。

## 功能特点

- **实时监测**：通过USB摄像头实时监测驾驶员行为
- **多种行为识别**：
  - 闭眼检测（疲劳驾驶预警）
  - 打哈欠检测（疲劳驾驶预警）
  - 喝水检测（分心驾驶预警）
  - 打电话检测（分心驾驶预警）
- **事件记录**：自动记录异常驾驶行为，包括时间戳和图像证据
- **可配置**：通过JSON配置文件灵活调整系统参数

## 系统架构

系统主要由以下几个组件组成：

1. **驾驶行为监测类 (DriverMonitor)**：负责从摄像头获取视频流并进行行为识别
2. **配置读取类 (ConfigReader)**：负责从JSON配置文件读取系统配置
3. **事件记录类 (EventLogger)**：负责记录检测到的异常驾驶行为
4. **消息处理类 (MessageHandler)**：负责消息的序列化和反序列化

## 依赖项

- C++17兼容的编译器
- OpenCV (用于图像处理和计算机视觉)
- dlib (用于人脸检测和特征点识别)
- nlohmann_json (用于JSON解析)
- CMake 3.10或更高版本

## 构建方法

```bash
# 创建并进入构建目录
mkdir -p build
cd build

# 配置CMake项目
cmake ..

# 编译项目
make

# 安装（可选）
sudo make install
```

## 运行方法

```bash
# 使用默认配置文件
./driver_monitor_system

# 指定配置文件
./driver_monitor_system /path/to/config.json
```

## 配置文件

系统使用JSON格式的配置文件，默认位于`config/config.json`。主要配置项包括：

```json
{
    "camera": {
        "device_id": 0,          // 摄像头设备ID
        "width": 640,            // 图像宽度
        "height": 480,           // 图像高度
        "fps": 30                // 帧率
    },
    "detection": {
        "ear_threshold": 0.25,   // 眼睛纵横比阈值
        "mar_threshold": 0.6,    // 嘴部纵横比阈值
        "eye_closed_frames": 3,  // 连续闭眼帧数阈值
        "yawning_frames": 5,     // 连续哈欠帧数阈值
        "drinking_frames": 3,    // 连续喝水帧数阈值
        "phone_calling_frames": 5 // 连续打电话帧数阈值
    },
    "alert": {
        "enable_sound": true,    // 是否启用声音警报
        "sound_volume": 80,      // 声音音量
        "display_warning": true, // 是否显示警告
        "log_events": true       // 是否记录事件
    },
    "model": {
        "face_landmark_model": "shape_predictor_68_face_landmarks.dat" // 面部特征点模型路径
    },
    "output": {
        "save_events": true,     // 是否保存事件
        "events_dir": "events",  // 事件目录
        "save_images": true,     // 是否保存图像
        "images_dir": "images"   // 图像目录
    }
}
```

## 使用说明

1. 启动程序后，系统会自动打开摄像头并开始监测驾驶行为
2. 当检测到异常行为时，系统会在屏幕上显示警告信息
3. 所有异常行为都会被记录到事件日志中，包括时间戳和图像证据
4. 按下'q'键或Ctrl+C可以退出程序

## 事件记录

系统会自动记录检测到的异常驾驶行为，包括：

- 行为类型
- 提示信息
- 时间戳
- 图像证据

事件日志保存在`events/events.log`文件中，图像证据保存在`images/`目录下。

## 注意事项

- 确保摄像头正常工作并且驱动已正确安装
- 系统需要下载dlib的面部特征点预测模型，首次运行可能需要一些时间
- 为了获得最佳效果，请确保光线充足，摄像头能够清晰捕捉到驾驶员的面部
