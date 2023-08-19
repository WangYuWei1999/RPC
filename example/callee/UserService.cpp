#include<iostream>
#include<string>
#include"../src/include/RpcApplication.hpp"
#include"../src/include/RpcLogger.hpp"
#include"User.pb.h"
#include"../src/include/RpcProvider.hpp"

//protobuf提供了login函数的接口，在UserService中继承UserServiceRpc类(继承自google::protobuf::Service)，重写login函数。
//UserService 是一个本地服务类，提供了两个本地方法，login 和 getfriendlist

class UserService : public ik::UserServiceRpc {
public:

    bool Login(std::string name, std::string password){
        std::cout<<"doing local service: lgoin: ";
        std::cout<<"name: "<< name << "password"<< password << std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string password){
        std::cout<<"doing local service: Register: ";
        std::cout<<"id: "<< id <<"name: "<< name <<"password"<< password<< std::endl;
        return true;
    }

    //重写基类UserServiceRpc函数，供框架调用
    void Login(google::protobuf::RpcController* controller,
                const ik::LoginRequest* request,
                ik::LoginResponse* response,
                google::protobuf::Closure* done)
    {
        //框架给业务上报了请求参数， 业务获取相应数据作本地业务
        std::string name =  request->name();
        std::string password =  request->password();

        //本地业务
        bool login_result = Login(name, password);

        //把响应给调用方返回
        ik::ErrorMsg* errmsg = response->mutable_errmsg();
        errmsg->set_error(0);
        errmsg->set_error_msg("");
        response->set_success(login_result);

        //执行回调操作
        done->Run();
    }

    void Register(google::protobuf::RpcController* controller,
                  const ik::RegisterRequest* request,
                  ik::RegisterResponse* response,
                  google::protobuf::Closure* done)
    {
        //框架给业务上报了请求参数， 业务获取相应数据作本地业务
        uint32_t id = request->id();
        std::string name = request->name();
        std::string password = request->password();

        //本地业务
        bool login_result = Register(id, name, password);

        //把响应给调用方
        ik::ErrorMsg* errmsg = response->mutable_error();
        errmsg->set_error(0);
        errmsg->set_error_msg("");
        response->set_success(login_result);

        //执行回调操作
        done->Run();
    }
};

int mian(int argc, char** argv){
    RPC_LOG_INFO("HELO");
    RPC_LOG_ERROR("%s,%s,%d", __FILE__, __FUNCTION__, __LINE__);

    //调用框架初始化操作
    RpcApplication::init(argc, argv);

    //框架服务提供provider
    RpcProvider provider;
    provider.notity_service(new UserService());  //将UserService服务提供的方法进行预发布
    provider.run();  //将预备发布的服务对象及方法注册到Zookeeper上并开启对远端调用的网络监听
    return 0;
}