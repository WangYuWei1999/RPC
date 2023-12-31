#include "RpcChannel.hpp"
#include "RpcApplication.hpp"
#include "RpcControl.hpp"
#include "RpcHeader.pb.h"
#include "RpcLogger.hpp"
#include"ZooKeeperClient.hpp"

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<unistd.h>
#include<errno.h>

#define BUFFER_SIZE 1024

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                            google::protobuf::RpcController *controller,
                            const google::protobuf::Message *request,
                            google::protobuf::Message *response,
                            google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor* service_des = method->service();
    std::string service_name = service_des->name();
    std::string method_name = method->name();

    //获取参数的序列化字符串长度 args_size
    int args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str)) args_size = args_str.size();  //参数序列化
    else {
        controller->SetFailed("serialize request error!");
        return;
    }

    //定义RPC请求header
    ikrpc::RpcHeader rpc_header; //头部包含了服务名，方法名，参数序列化后的长度
    rpc_header.set_service_name(service_name);
    rpc_header.set_method_name(method_name);
    rpc_header.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;  //序列化后头部
    if(rpc_header.SerializeToString(&rpc_header_str)) {
        //序列化成功
        header_size = rpc_header_str.size();
    }else {
        //序列化失败 
        controller->SetFailed("serialize rpc_header error");
        return;
    }

    //组织待发送的rpc请求字符串 包括序列化后头部长度，序列化后头部，序列化后参数
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size,4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    //使用tcp编程，完成rpc方法远程调用
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd = -1){
        close(client_fd);
        RPC_LOG_FATAL("create socke error, errno:%d", errno);
    }

    //获取ip和端口
    ZookeeperClient zk_client;
    zk_client.start();         //zk初始化
    std::string method_path = "/" + service_name + "/" + method_name;    //利用服务名加方法名查询
    std::string host_data = zk_client.get_data(method_path.c_str());     //在zk查询到ip端口
    if(host_data == ""){
        controller->SetFailed(method_path+"is not exist");
        return;
    }
    int host_index = host_data.find(":");
    if(host_index == -1){
        controller->SetFailed(method_path + "address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0, host_index);  //取出ip
    uint16_t port = atoi(host_data.substr(host_index+1, host_data.size()-host_index).c_str());   //取出端口 int atoi(const char*);字符串转整数
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    //建立连接
    if(connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))== -1){
        close(client_fd);
        RPC_LOG_FATAL("connect error! errno: %d", errno);
    }

    //发送rpc请求
    if(send(client_fd, send_rpc_str.c_str(), send_rpc_str.size(), 0) ==-1){
        controller->SetFailed("send error, :"+errno);
        close(client_fd);
        return;
    }

    //整个过程同步，发送rpc请求后直接阻塞等待rpc响应返回

    //接受rpc请求
    char recv_buffer[BUFFER_SIZE] = {0};
    ssize_t recv_size  = recv(client_fd, recv_buffer, BUFFER_SIZE, 0);
    if(recv_size == -1){
        controller->SetFailed("recv errno, errno: "+ errno);
        close(client_fd);
        return;
    }

    //反序列化响应数据
    //string 因为遇到\0会认为是字符串结束，所以使用array
    if(!response->ParseFromArray(recv_buffer, recv_size)){
        std::string recv = recv_buffer;
        controller->SetFailed("parse errro, response_str: "+recv);
        close(client_fd);
        return;
    }
    close(client_fd);
}