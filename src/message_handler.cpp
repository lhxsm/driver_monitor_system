#include "../include/message_handler.hpp"
#include <iostream>
#include <cstring>
#include <cstdint>

FunctionType Message::stringToFunctionType(const std::string& func_str) {
    if (func_str == "DMS") {
        return FunctionType::DMS;
    } else {
        return FunctionType::UNKNOWN;
    }
}

std::string Message::functionTypeToString(FunctionType func_type) {
    switch (func_type) {
        case FunctionType::DMS:
            return "DMS";
        default:
            return "UNKNOWN";
    }
}

DataType Message::stringToDataType(const std::string& data_type_str) {
    if (data_type_str == "IMAGE") {
        return DataType::IMAGE;
    } else if (data_type_str == "TEXT") {
        return DataType::TEXT;
    } else if (data_type_str == "INFO") {
        return DataType::INFO;
    } else {
        return DataType::UNKNOWN;
    }
}

std::string Message::dataTypeToString(DataType data_type) {
    switch (data_type) {
        case DataType::IMAGE:
            return "IMAGE";
        case DataType::TEXT:
            return "TEXT";
        case DataType::INFO:
            return "INFO";
        default:
            return "UNKNOWN";
    }
}

std::vector<uint8_t> MessageHandler::serializeMessage(const Message& msg) {
    // 序列化消息格式：
    // [功能类型(字符串)]\0[数据类型(字符串)]\0[数据]
    
    std::string func_str = Message::functionTypeToString(msg.function);
    std::string data_type_str = Message::dataTypeToString(msg.data_type);
    
    // 计算序列化后的总大小
    size_t total_size = func_str.size() + 1 + data_type_str.size() + 1 + msg.data.size();
    
    // 创建结果向量
    std::vector<uint8_t> result;
    result.reserve(total_size);
    
    // 添加功能类型
    result.insert(result.end(), func_str.begin(), func_str.end());
    result.push_back('\0');
    
    // 添加数据类型
    result.insert(result.end(), data_type_str.begin(), data_type_str.end());
    result.push_back('\0');
    
    // 添加数据
    result.insert(result.end(), msg.data.begin(), msg.data.end());
    
    return result;
}

Message MessageHandler::deserializeMessage(const std::vector<uint8_t>& data) {
    Message msg;
    
    try {
        // 查找第一个分隔符位置
        auto first_null = data.end();
        for (auto it = data.begin(); it != data.end(); ++it) {
            if (*it == 0) { // 查找值为0的字节，即'\0'
                first_null = it;
                break;
            }
        }
        
        if (first_null == data.end()) {
            throw std::runtime_error("无效的消息格式：缺少第一个分隔符");
        }
        
        // 提取功能类型字符串
        std::string func_str(data.begin(), first_null);
        msg.function = Message::stringToFunctionType(func_str);
        
        // 查找第二个分隔符位置
        auto second_null = data.end();
        for (auto it = first_null + 1; it != data.end(); ++it) {
            if (*it == 0) { // 查找值为0的字节，即'\0'
                second_null = it;
                break;
            }
        }
        
        if (second_null == data.end()) {
            throw std::runtime_error("无效的消息格式：缺少第二个分隔符");
        }
        
        // 提取数据类型字符串
        std::string data_type_str(first_null + 1, second_null);
        msg.data_type = Message::stringToDataType(data_type_str);
        
        // 提取数据
        msg.data.assign(second_null + 1, data.end());
        
    } catch (const std::exception& e) {
        std::cerr << "反序列化消息失败: " << e.what() << std::endl;
        msg.function = FunctionType::UNKNOWN;
        msg.data_type = DataType::UNKNOWN;
        msg.data.clear();
    }
    
    return msg;
}
