#include<iostream>
#include"../src/include/RpcApplication.hpp"
#include"../src/include/RpcControl.hpp"
#include"../include/User.pb.h"
#include"../src/include/RpcChannel.hpp"

int main(int argc, char** argv){
    //初始化ip和端口
    RpcApplication::init(argc, argv);

    //调用远程发布的rpc方法login
    ik::UserServiceRpc_Stub stub (new RpcChannel());

    //请求参数和响应
    ik::LoginRequest request;
    request.set_name("zhang san");
    request.set_password("123456");
     
    ik::LoginResponse response;
    
    //发起rpc调用， 等待响应结果
    stub.Login(nullptr, &request, &response, nullptr);

    //rpc调用完成，读调用结果
    if(response.errmsg().error() == 0){
        //没错误
        std::cout<<"rpc login response: " << response.success() << std::endl;
    }else{
        //有错误
        std::cout<<"rpc login error: "<<response.errmsg().error_msg()<< std::endl;
    }

    ik::RegisterRequest reg_request;
    reg_request.set_id(2000);
    reg_request.set_name("rpc");
    reg_request.set_password("123456");
    ik::RegisterResponse reg_response;

    //rpc调用完成，读取调用结果
    if(reg_response.error().error() == 0){
        //没错误
        std::cout<<"rpc regiseter response: "<< reg_response.success()<<std::endl;
    }else{
        //有错
        std::cout<<"rpc register response error:"<<reg_response.error().error_msg()<<std::endl;
    }

    return 0;
}