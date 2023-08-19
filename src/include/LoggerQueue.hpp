#pragma once

#include <queue>
#include<mutex>
#include<thread>
#include<condition_variable>

//异步写入日志的队列
template<typename T>
class LoggerQueue{
public:
    void push(const T& data){
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(data);
        condition_.notify_one();
    }

    T pop(){
        std::unique_lock<std::mutex> lock(mutex_);
        while(queue_.empty()){
            condition_.wait(lock); //队列为空则阻塞并解锁
        }
        T ret = queue_.front();
        queue_.pop();
        return ret;
    }
    
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable condition_;
};