#pragma once

#include<google/protobuf/service.h>
#include<string>

class RpcControl : public google::protobuf::RpcController{
public:
    RpcControl();
    void Reset();

    bool Failed() const;

    std::string ErrorText() const;

    void SetFailed(const std::string& reason);

public:
    void StartCancel();

    bool IsCanceled() const;
    
    void NotifyOnCancel(google::protobuf::Closure* callback);

private:
    bool failed_; //Rpc 方法执行过程中的状态
    std::string error_text_; //Rpc方法执行过程中的错误信息
};