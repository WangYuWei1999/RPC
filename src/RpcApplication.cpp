#include"RpcApplication.hpp"
#include<muduo/base/Logging.h>
#include<iostream>
#include<unistd.h>
#include<string>
using namespace muduo;

RpcConfigure RpcApplication::configure_;   //饿汉模式 静态变量类中定义，类外初始化

//首先从输入参数中解析出文件名，然后加载配置文件
void RpcApplication::init(int argc, char** argv)
{
    if(argc<2) LOG_FATAL<<"format: command -i <configfile>";
     
     int choose = 0;
     std::string config_file;

     //getopt方法分析命令行参数 "i:" 其中i后面有冒号,所以后面必须有参数
     while((choose = getopt(argc, argv, "i:")) != -1){
        switch(choose){
            case 'i':
                config_file = optarg;
                break;
            case '?':
                LOG_FATAL<<" format:command -i <configfile>";
                break;
            case ':':
                LOG_FATAL<<" format:command -i <configfile>";
                break;
            default:
                break;
        }
    }
    //加载配置文件
    configure_.load_configure(config_file.c_str());    
}

RpcApplication::RpcApplication(){}