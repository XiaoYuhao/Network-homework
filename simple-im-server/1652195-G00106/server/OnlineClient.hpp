
#ifndef _CLIENT_OPERATING_

#define _CLIENT_OPERATING_

#include <vector>
#include <string>
#include <time.h>

#include "package.hpp"

const time_t TIMEOUT = 50; // 默认秒，单位待定

const int MAX_BUFFER_SIZE = 4096; // 缓冲区最大容量

using std::string;

class OnlineClient
{
    /*
     * server端处理单个连接所有操作的类
     * 包含客户端的描述字、产生的随机数、当前状态和收发的数据等
     * 在每次server端监听到一个新的客户端连接时产生对象
     */
private:
    /*
     * 所有在与客户端交互时所需的辅助数据
     */
    static int count; // 当前创建的对象个数
    string client_ip; // 客户端ip
    time_t timer = 0; // 超时计时器

    char *read_buffer;    // 读缓冲区，
    int read_buf_pointer; // 读缓冲区指针
    char *write_buffer;   //
    int write_buf_pointer;

public:
    int sock;             // 本客户端的描述字
    short userid = -1;     // 此客户端登录的用户-id
    bool dead = false;

private:
    /* 
     * 客户端本身需要的信息
     * 包括连接是否成功、上次发送心跳包时间等
     */


public:
    OnlineClient(int sock)
    {
        this->sock = sock;
        read_buffer = new char[MAX_BUFFER_SIZE];
        write_buffer = new char[MAX_BUFFER_SIZE];
    }
    ~OnlineClient()
    {
        delete[] read_buffer;
        delete[] write_buffer;
    }

    void refresh_timer()
    {
        this->timer = 0.f;
    }

    /*
    * Server类定时调用本方法，更新所有在线用户的计时器
    * 一旦计时器的值超过限定则认为超时，返回false
    */
    bool update_timer(time_t time_passed)
    {
        this->timer += time_passed;
        if (this->timer >= TIMEOUT)
            return false;
        else
            return true;
    }

    // bool is_client_login()
    // {
    //     return is_login;
    // }
    // short login(string username, string password);
};

#endif