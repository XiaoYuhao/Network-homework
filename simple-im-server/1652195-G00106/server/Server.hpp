#ifndef _SERVER_HEADER_

#define _SERVER_HEADER_

#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <exception>
#include <map>
#include <set>
#include <utility>
#include <functional>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>

#include "OnlineClient.hpp"
#include "Database.hpp"
#include "package.hpp"
#include "tools.hpp"

#define QUEUE_SIZE 12

// using namespace std;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::ofstream;
using std::string;
using std::map;
using std::set;
using std::make_pair;

class Server{
    /*
     * Server主类，在一个程序中生成一个对象即可
     * 负责Server处理同其他client连接的各类操作
     * 以及与本地数据库/硬盘其他配置文件的交互
     */
private:
    map<int, OnlineClient *> sock_map_client;  // 存放客户端对象地址的哈希表，映射关系：fd->client
    map<int, OnlineClient *> id_map_client;
    vector<OnlineClient *> online_clients;          // 存储所有已连接客户端的容器 如果断开连接则移除元素

    Database database;                         // 数据库类database的实例化对象
    int server_port;                                   // 端口号信息
    int server_sock;                                // 服务器socket描述字

    char temp_log[1024];
    ofstream log_file_stream;                    // 写日志文件的文件流，日志文件每次运行服务器都会生成，且以开始运行时间作为关键字

private:
    void print_error_log(string err_log)
    {    // 打印错误信息，统一错误输出方法
        log_file_stream << "ERROR: " << get_current_time() << ": " << err_log
                        << " - " << strerror(errno) << endl;
        cout << "ERROR: " << get_current_time() << ": " << err_log << endl;
    }
    void print_log(string log) 
    {
        log_file_stream << get_current_time() << ": " << log << endl;
    }
    void print_hint(string hint)
    {
        cout << hint << endl;
    }

public:
    Server(int server_portNum = 23100);            // 构造函数用来构建日志文件流、记录端口号
    ~Server() {
        // 析构函数释放所有single_client的内存
        for(auto i = sock_map_client.begin();i != sock_map_client.end();++i) {
            delete i->second;
            i->second = NULL;
        }

        print_log("Server closed.");
        log_file_stream.close();
    }
    Status accept_new_client(fd_set *p_rfd, int &maxfd);
    void quit_client(OnlineClient *client_to_delete);    // 客户端断开连接后进行的操作

    Status alive_confirm(OnlineClient * sender_client);

    Status login_req(OnlineClient * sender_client, char func_byte);
    Status user_login(OnlineClient * sender_client);
    Status change_password(OnlineClient * sender_client);

    Status transfer_message(OnlineClient * sender_client, int pack_length);
    Status transfer_filereq(OnlineClient * sender_client, char func_byte, int pack_length);
    Status transfer_file(OnlineClient * sender_client, int pack_length);


    Status send_config(OnlineClient * sender_client);         // 发送配置信息到客户端的操作
    Status history_req(OnlineClient * sender_client);

    void init_socket();                      // 初始化server的socket，包括socket/bind/listen方法
    void server_mainloop();                    // Server程序正常运行在本方法中并进行各类操作
};

#endif