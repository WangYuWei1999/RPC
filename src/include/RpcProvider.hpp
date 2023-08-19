#pragma once
#include<google/protobuf/service.h>
#include<google/protobuf/descriptor.h>

#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/TcpConnection.h>
#include<muduo/net/Buffer.h>

#include<unordered_map>
#include<string>
#include<iostream>
#include<functional>

//提供专门负责发布rpc服务的网络框架
class RpcProvider{
public:
    //框架提供外部接口，可以发布rpc方法的函数接口
    void notity_service(google::protobuf::Service* service);

    //启动rpc服务节点，开始提供rpc远程网络调用服务
    void run();

private:
    //新socket连接的回调
    void on_connection(const muduo::net::TcpConnectionPtr& conn);

    //已建立连接的读写事件回调
    void on_message(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer, muduo::Timestamp stamp);

    //clouser的回调，用于序列化rpc的响应的网络发送
    void send_rpc_response(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* reponse);

private:
    //服务类型信息
    struct ServiceInfo{
        //保存服务对象
        google::protobuf::Service* service_;
        //保存服务方法（某个服务提供对象的所有方法）
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor* > method_map_;
    };

private:
    muduo::net::EventLoop eventloop_;
    //存储服务信息(服务提供对象及其提供的方法)
    std::unordered_map<std::string, ServiceInfo> service_map_;
};