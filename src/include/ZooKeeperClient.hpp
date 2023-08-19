#pragma once

#include<semaphore.h>
#include<zookeeper/zookeeper.h>
#include<string>

class ZookeeperClient{
public:
    ZookeeperClient();
    ~ZookeeperClient();

    //启动连接
    void start();

    //在skserver根据指定path创建znode节点
    void create(const char* path, const char* data, int datalen, int state = 0);

    //根据参数指定的znode节点路径获取znode节点的值
    std::string get_data(const char* path);

private:
    //zk客户端句柄
    zhandle_t* zhandle_ = nullptr;

};