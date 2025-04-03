#pragma once

#include <string>
#include <vector>
#include <memory>

// 功能类型枚举
enum class FunctionType {
    DMS,        // 驾驶员监控系统
    UNKNOWN
};

// 数据类型枚举
enum class DataType {
    IMAGE,      // 图像数据
    TEXT,       // 文本数据
    INFO,       // 信息数据
    UNKNOWN
};

// 消息结构体
struct Message {
    FunctionType function;
    DataType data_type;
    std::vector<uint8_t> data;
    
    static FunctionType stringToFunctionType(const std::string& func_str);
    static std::string functionTypeToString(FunctionType func_type);
    static DataType stringToDataType(const std::string& data_type_str);
    static std::string dataTypeToString(DataType data_type);
};

// 消息处理类
class MessageHandler {
public:
    MessageHandler() = default;
    ~MessageHandler() = default;
    
    // 序列化消息
    static std::vector<uint8_t> serializeMessage(const Message& msg);
    
    // 反序列化消息
    static Message deserializeMessage(const std::vector<uint8_t>& data);
};
