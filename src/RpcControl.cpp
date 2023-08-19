#include"RpcControl.hpp"

RpcControl::RpcControl(){
    failed_ = false;
    error_text_ = "";
}

void RpcControl::Reset(){
    failed_ = false;
    error_text_ = "";
}

bool RpcControl::Failed() const{
    return failed_;
}

std::string RpcControl::ErrorText() const{
    return error_text_;
}

void RpcControl::SetFailed(const std::string& reason){
    failed_ = true;
    error_text_ = reason;
}

//暂不实现
void RpcControl::StartCancel(){}

bool RpcControl::IsCanceled() const {return false;}

void RpcControl::NotifyOnCancel(google::protobuf::Closure* callback){}