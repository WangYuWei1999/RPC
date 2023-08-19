#include"RpcConfigure.hpp"
#include"RpcLogger.hpp"
#include<muduo/base/Logging.h>
#include<string>
#include<iostream>

#define BUFFER_SIZE 128

//加载配置文件
void RpcConfigure::load_configure(const char* config_file){
    FILE* pf = fopen(config_file, "r");
    if(pf ==nullptr) RPC_LOG_FATAL("%s is not exit!", config_file);

    //每次解析一行
    while(!feof(pf)){
        char buf[BUFFER_SIZE] = {0};
        //char *fgets(char *str, int n, FILE *stream) 从指定的流 stream 读取一行，并把它存储在 str 所指向的字符串内
        fgets(buf, BUFFER_SIZE, pf);

        std::string str_buf(buf);

        trim(str_buf);  //去掉前后空格

        //判断 注释# 空行 直接跳过
        if(str_buf[0] == '#' || str_buf[0] == '\n' || str_buf.empty()) continue;
        
        //解析配置
        int index = str_buf.find('=');
        if(index==-1){
            RPC_LOG_ERROR("configure file illegal!");
            continue;
        }

        std::string key = str_buf.substr(0, index);
        trim(key);

        std::string value = str_buf.substr(index+1, str_buf.size()-index);
        value[value.size()-1] = ' ';   //去除最后一个换行符号'\n'
        trim(value);

        configure_map_.insert({key,value});
    }
}

//查询配置信息
std::string RpcConfigure::find_load(std::string key){
    auto it = configure_map_.find(key);
    if(it == configure_map_.end()) return "";
    return it->second;
}

//去掉字符串前后的空格
void RpcConfigure::trim(std::string& str_buf){
    //找到第一个不为空格的字符
    int index = str_buf.find_first_not_of(' ');
    if(index!=-1) str_buf = str_buf.substr(index,str_buf.size()-index); //截取字符串，去掉前面空格
    
    index = str_buf.find_last_not_of(' ');
    if(index != -1) str_buf = str_buf.substr(0, index+1);
}