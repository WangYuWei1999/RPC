#include"RpcProvider.hpp"
#include"RpcApplication.hpp"
#include"RpcHeader.pb.h"
#include"RpcLogger.hpp"
#include"ZooKeeperClient.hpp"

//using namespace std;
using namespace std::placeholders;

/*
service_name => service 描述
                service* 记录服务对象
                method_name => 对于method发方法对象
*/
//框架提供给外部使用的，可以发布rpc方法的函数接口;
//遍历服务对象方法，储存在ServiceInfo结构体中
void RpcProvider::notity_service(google::protobuf::Service* service){
    ServiceInfo service_info;
    //借助google::protobuf::Service获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor* service_desc = service->GetDescriptor();

    //获取服务对象名字
    std::string service_name = service_desc->name();

    //获取服务对象方法的数量
    int method_nums = service_desc->method_count();

    for(int i=0;i<method_nums;++i){
        //获取服务对象指定下标的服务描述
        const google::protobuf::MethodDescriptor* method_desc = service_desc->method(i);
        std::string method_name = method_desc->name();
        //插入信息
        //service_info.method_map_.insert(std::make_pair(method_name, method_desc));
        service_info.method_map_.insert({method_name, method_desc});
    }
    service_info.service_ = service;
    service_map_.insert({service_name, service_info});
}

//启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::run(){
    //获取本地ip和端口
    std::string ip = RpcApplication::get_instance().get_configure().find_load("rpcserver_ip");
    uint16_t port = atoi(RpcApplication::get_instance().get_configure().find_load("rpcserver_port").c_str());
    muduo::net::InetAddress address(ip, port);

    //创建tcpserver对象
    muduo::net::TcpServer server(&eventloop_, address, "RpcProvider");
    //绑定链接回调和消息读写回调方法
    //被std::function<void(const TcpConnectionPtr&)> connectonCallback接收
    server.setConnectionCallback(std::bind(&RpcProvider::on_connection, this, _1));
    //被std::function<vid(const TcpConnectionPtr&, Buffer*, Timestamp)> messageCallback 接受
    server.setMessageCallback(std::bind(&RpcProvider::on_message, this, _1, _2, _3));

    //设置muduo库的线程数量
    server.setThreadNum(4);

    //把当前rpc节点上要发布的服务全部注册到zk上，rpc_client可以从zk上发现服务
    ZookeeperClient zk_client;
    zk_client.start();

    //在配置中心创建节点(遍历所有服务对象)
    //同一个服务的所有方法ip地址都一样
    for(auto& sp:service_map_){
        std::string service_path = "/" + sp.first;          //服务名称
        zk_client.create(service_path.c_str(), nullptr, 0);
        for(auto& mp:sp.second.method_map_){                    //遍历某个对象提供的所有方法
            std::string method_path = service_path + "/" + mp.first;   //提供服务对象名称+‘/’+方法名称
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);   //将ip地址：端口写入method_path_data
            //ZOO_EPHEMERAL 表示znode时候临时性节点，将方法和对于ip端口注册到zk上
            zk_client.create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    RPC_LOG_INFO("server RpcProvider [ip: %s][port: %d]", ip.c_str(), port);
    server.start();
    eventloop_.loop();
}


//新socket连接的回调
void RpcProvider::on_connection(const muduo::net::TcpConnectionPtr& conn){
    if(!conn->connected()){
        //和rpc_client断开连接
        conn->shutdown();
    }
}

/*
    在框架内部，RpcPrivoder和RpcConsumer协商好之间通信用的protobuf类型
标识长度：header_size = service_name + method_name + argc_size
    service_name ==> service
    method_name ==> method
    定义proto的message类型：
        args

recv_buf = header + args
16UserServiceLogin15zhang san123456

--> header_size（4字节）+ header_str + args_str
*/

//已建立连接的读写事件回调
void RpcProvider::on_message(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer, muduo::Timestamp stamp){
    //网络上接受的远程rpc调用请求字符流
    std::string recv_buf = buffer->retrieveAllAsString();  //反序列化

    //从字节流中读取前四个字符的内容（利用int特性），即header长度
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4,0);  //前四个字节拷贝到header_size中

    RPC_LOG_INFO("header size: %d", header_size);

    //根据header_size读取数据头的原始字符流
    std::string rpc_header_str = recv_buf.substr(4, header_size);

    //反序列化
    ikrpc::RpcHeader rpc_header;
    std::string service_name;
    std::string method_name;
    uint32_t args_size = 0;
    if(rpc_header.ParseFromString(rpc_header_str)){
        //数据头反序列化成功
        service_name = rpc_header.service_name();
        method_name = rpc_header.method_name();
        args_size = rpc_header.args_size();
    }else{
        //数据头反序列化失败
        RPC_LOG_ERROR("rpc header str:: %s parse error!", rpc_header_str.c_str());
        return;
    }

    //解析参数
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    //打印调试信息
    RPC_LOG_INFO("<-------------------------------------------------------------->");
    RPC_LOG_INFO("rpc header str: %s",rpc_header_str.c_str());
    RPC_LOG_INFO("service name: %s", service_name.c_str());
    RPC_LOG_INFO("method name: %s", method_name.c_str());
    RPC_LOG_INFO("args: %s", args_str.c_str());
    RPC_LOG_INFO("<-------------------------------------------------------------->");

    //获取service 对象和method 方法
    auto service_it = service_map_.find(service_name);
    if(service_it == service_map_.end()){
        RPC_LOG_ERROR("%s is not exist", service_name.c_str());
        return;
    }

    auto method_it = service_it->second.method_map_.find(method_name);
    if(method_it == service_it->second.method_map_.end()){
        RPC_LOG_ERROR("%s::%s is not exist", service_name.c_str(), method_name.c_str());
        return;
    }

    //获取service和method对象
    google::protobuf::Service* service = service_it->second.service_;
    const google::protobuf::MethodDescriptor* method = method_it->second;

    //生成rpc方法调用的请求request    和 响应response参数
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str)){
        RPC_LOG_ERROR("request parse error!");
        return;
    }

    //定义一个空的响应response 用于下面的函数service->CallMethod(), 将caller请求的结果填写进去
    google::protobuf::Message* response = service->GetResponsePrototype(method).New();

    //给callMethod方法的调用，绑定一个closure函数 闭包
    //闭包捕获了一个成员对象的成员函数及其所需的实参。闭包提供了方法run(),执行run()函数，即捕获到成员对象的成员函数，
    //即相当于执行void RpcProvider::send_rpc_response(conn, response)；
    google::protobuf::Closure *done = 
            google::protobuf::NewCallback<RpcProvider, 
                                        const muduo::net::TcpConnectionPtr&, 
                                        google::protobuf::Message*>
                                        (this, &RpcProvider::send_rpc_response, conn, response);   //this成员对象的函数及其参数

    //在框架根据远端rpc请求，调用响应方法
    //google::protobuf::Service中纯虚函数CallMethod被UserServiceRpc实现，UserService继承自UserServiceRpc
    //根据传递的方法描述符method，选择对应的函数。这里会有多态发生，会调用UserService中的函数（login）
    service->CallMethod(method, nullptr, request, response, done);  //service是我们的UserService, CallMethod是Service基类中的函数
}

//Clouser的回调操作， 用于序列化rpc的响应和网络发送，即序列化respnse
void RpcProvider::send_rpc_response(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response){
    std::string response_str;
    //序列化
    if(response->SerializeToString(&response_str)){
        //发送序列化数据
        conn->send(response_str);
    }else{
        //序列化失败
        RPC_LOG_ERROR("serialize response error");
    }
    //短连接
    conn->shutdown();
}
