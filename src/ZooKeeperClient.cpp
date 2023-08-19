#include"ZooKeeperClient.hpp"
#include"RpcApplication.hpp"
#include"RpcLogger.hpp"
#include<iostream>


void global_watcher(zhandle_t* handler, int type, int state, const char* path, void* watcher_context){
    if(type == ZOO_SESSION_EVENT) {
        //连接成功就会发送成功信号
        if(state == ZOO_CONNECTED_STATE){  //回调消息类型
            //获取信号量
            sem_t* sem = (sem_t*)zoo_get_context(handler);  //zkclient 和zkserver连接成功
            sem_post(sem);
        }
    }
}

ZookeeperClient::ZookeeperClient(){}

ZookeeperClient::~ZookeeperClient(){
    if(zhandle_ != nullptr) zookeeper_close(zhandle_);
}

//启动连接
void ZookeeperClient::start(){
    //通过rpcapplication-》rpcconfigure->查询连接信息（ip和端口）
    std::string host = RpcApplication::get_instance().get_configure().find_load("zookeeper_ip");
    std::string port = RpcApplication::get_instance().get_configure().find_load("zookeeper_port");
    std::string con_str = host + ":" + port;
    std::cout<<con_str<<std::endl;
    zhandle_ = zookeeper_init(con_str.c_str(), global_watcher, 30000, nullptr, nullptr,0);
    if(zhandle_ == nullptr) RPC_LOG_ERROR("zookeeper init error");

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(zhandle_, &sem);  //设置信号量

    sem_wait(&sem);
    RPC_LOG_INFO("zookeeper init success");
}

//在zkserver 根据指定的path创建znode节点
void ZookeeperClient::create(const char* path, const char* data, int datalen, int state){
    char path_buffer[128] = {0};
    int buffer_len = sizeof(path_buffer);
    int flag;

    //同步检查path是否存在
    flag = zoo_exists(zhandle_, path, 0, nullptr);
    if(flag == ZNONODE){  //不存在
        flag = zoo_create(zhandle_, path,data, datalen, &ZOO_OPEN_ACL_UNSAFE, state, path_buffer,buffer_len);
    }

    if(flag == ZOK) //成功 
    {
        RPC_LOG_INFO("znode create success  path: %s",path);
    }
    else  RPC_LOG_FATAL("falg: %d, znode create error  path: %s", flag, path);
}

//根据参数指定的znode节点路径，获取znode节点的值
std::string ZookeeperClient::get_data(const char* path){
    char buffer[64] = {0};  //存储返回结果
    int buffer_len = sizeof(buffer);
    int flag = zoo_get(zhandle_, path, 0, buffer, &buffer_len, nullptr);
    if (flag != ZOK){
        RPC_LOG_ERROR("cant get znode path: %s", path);
        return"";
    }else{
        return buffer;
    }
}
