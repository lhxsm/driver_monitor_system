cmake_minimum_required(VERSION 3.10)
project(driver_monitor_system)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找OpenCV库
find_package(OpenCV REQUIRED)

# 查找dlib库
find_package(dlib QUIET)
if(NOT dlib_FOUND)
    # 如果没有找到dlib，可以使用FetchContent下载
    include(FetchContent)
    FetchContent_Declare(
        dlib
        GIT_REPOSITORY https://github.com/davisking/dlib.git
        GIT_TAG v19.24
    )
    set(DLIB_USE_CUDA OFF CACHE BOOL "Use CUDA")
    set(DLIB_NO_GUI_SUPPORT ON CACHE BOOL "No GUI support")
    set(DLIB_ENABLE_ASSERTS OFF CACHE BOOL "Enable asserts")
    FetchContent_MakeAvailable(dlib)
endif()

# 查找nlohmann_json库
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    # 如果没有找到nlohmann_json，可以使用FetchContent下载
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# 添加包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OpenCV_INCLUDE_DIRS})

# 添加源文件
set(SOURCES
    src/main.cpp
    src/driver_monitor.cpp
    src/config_reader.cpp
    src/event_logger.cpp
    src/message_handler.cpp
)

# 创建可执行文件
add_executable(driver_monitor_system ${SOURCES})

# 链接库
target_link_libraries(driver_monitor_system ${OpenCV_LIBS} dlib::dlib pthread)
if(nlohmann_json_FOUND)
    target_link_libraries(driver_monitor_system nlohmann_json::nlohmann_json)
endif()

# 安装目标
install(TARGETS driver_monitor_system DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/config/ DESTINATION etc/driver_monitor_system)

# 添加一个自定义目标，用于复制配置文件到构建目录
add_custom_target(copy_configs ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_BINARY_DIR}/config
    COMMENT "Copying config files to build directory"
)

# 添加一个自定义目标，用于下载dlib面部特征点预测模型
add_custom_target(download_model ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/models
    COMMAND ${CMAKE_COMMAND} -E echo "Downloading dlib face landmark model..."
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR}/models curl -L -O http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR}/models bunzip2 -f shape_predictor_68_face_landmarks.dat.bz2
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/models/shape_predictor_68_face_landmarks.dat ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Downloading dlib face landmark model"
)

# 创建事件和图像目录
add_custom_target(create_dirs ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/events
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/images
    COMMENT "Creating events and images directories"
)
