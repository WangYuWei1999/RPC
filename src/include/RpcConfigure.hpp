#pragma once
#include<unordered_map>
#include<string>

//框架读取配置文件类
//rpcserver:ip rpcserver:port
//zookeeper:ip zookeeper:port

class RpcConfigure{
public:
    //加载配置文件
    void load_configure(const char* config_file);

    //查询配置信息
    std::string find_load(std::string key);

    //去掉字符串前后的空格
    void trim(std::string& str_buf);
private:
    std::unordered_map<std::string, std::string> configure_map_;  //存储配置信息
};