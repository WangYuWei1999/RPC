#include"RpcLogger.hpp"
#include<muduo/base/Logging.h>
#include<unistd.h>

#define FILENAME_SIZE 128
#define TIME_BUFFER 128

void RpcLogger::log_info(std::string msg){
    set_level(INFO);
    log(msg);
}

void RpcLogger::log_error(std::string msg){
    set_level(ERROR);
    log(msg);
}

void RpcLogger::log_fatal(std::string msg){
    set_level(FATAL);   
    log(msg);
    LOG_ERROR<<"Force Majeure is being dealt with";
    LOG_ERROR<<"Question has been record in log file, please restore system,bye";
    exit(0);
}

//设置日志级别
void RpcLogger::set_level(LogLevel level){
    log_level_ = level;
}

//写日志(将日志添加头部加入队列)
void RpcLogger::log(std::string msg){
    std::string begin_info;
    switch(log_level_){
        case INFO:
            begin_info = "[INFO]: ";
            break;
        case ERROR:
            begin_info = "[ERROR]: ";
            break;
        case FATAL:
            begin_info = "[FATAL]: ";
            break;
        default:
            break;
    }
    begin_info += msg;
    log_queue_.push(begin_info);
}

//启动写日志线程，负责写日志
RpcLogger::RpcLogger(){
    //创建写日志线程
    std::thread write_thread([&](){
        for(;;){
            //获取当前日期
            time_t now = time(nullptr);
            tm* now_time = localtime(&now);
            
            char file_name[FILENAME_SIZE] = {0};
            //sprintf将字符串打印到字符串中
            sprintf(file_name, "%d-%d-%d_log.txt", now_time->tm_year+1900, now_time->tm_mon+1, now_time->tm_mday);

            FILE* file_ptr = fopen(file_name, "a+");
            if(file_ptr == nullptr){
                LOG_ERROR<<"logger file: "<<file_name<<" open error";
            }

            std::string msg;
            char time_buf[TIME_BUFFER] = {0};
            sprintf(time_buf, "%2d:%2d:%2d=>", now_time->tm_hour, now_time->tm_min, now_time->tm_sec);
            msg = log_queue_.pop();
            msg.insert(0, time_buf);
            msg +='\n';
            //fputs(const char* str, FILE* stream)把字符串写入到指定的流中，不包括空字符
            fputs(msg.c_str(), file_ptr);
            fclose(file_ptr);
        }
    });

    //设置分离线程
    write_thread.detach();
}