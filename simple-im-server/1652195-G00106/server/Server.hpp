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
     * Server���࣬��һ������������һ�����󼴿�
     * ����Server����ͬ����client���ӵĸ������
     * �Լ��뱾�����ݿ�/Ӳ�����������ļ��Ľ���
     */
private:
    map<int, OnlineClient *> sock_map_client;  // ��ſͻ��˶����ַ�Ĺ�ϣ��ӳ���ϵ��fd->client
    map<int, OnlineClient *> id_map_client;
    vector<OnlineClient *> online_clients;          // �洢���������ӿͻ��˵����� ����Ͽ��������Ƴ�Ԫ��

    Database database;                         // ���ݿ���database��ʵ��������
    int server_port;                                   // �˿ں���Ϣ
    int server_sock;                                // ������socket������

    char temp_log[1024];
    ofstream log_file_stream;                    // д��־�ļ����ļ�������־�ļ�ÿ�����з������������ɣ����Կ�ʼ����ʱ����Ϊ�ؼ���

private:
    void print_error_log(string err_log)
    {    // ��ӡ������Ϣ��ͳһ�����������
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
    Server(int server_portNum = 23100);            // ���캯������������־�ļ�������¼�˿ں�
    ~Server() {
        // ���������ͷ�����single_client���ڴ�
        for(auto i = sock_map_client.begin();i != sock_map_client.end();++i) {
            delete i->second;
            i->second = NULL;
        }

        print_log("Server closed.");
        log_file_stream.close();
    }
    Status accept_new_client(fd_set *p_rfd, int &maxfd);
    void quit_client(OnlineClient *client_to_delete);    // �ͻ��˶Ͽ����Ӻ���еĲ���

    Status alive_confirm(OnlineClient * sender_client);

    Status login_req(OnlineClient * sender_client, char func_byte);
    Status user_login(OnlineClient * sender_client);
    Status change_password(OnlineClient * sender_client);

    Status transfer_message(OnlineClient * sender_client, int pack_length);
    Status transfer_filereq(OnlineClient * sender_client, char func_byte, int pack_length);
    Status transfer_file(OnlineClient * sender_client, int pack_length);


    Status send_config(OnlineClient * sender_client);         // ����������Ϣ���ͻ��˵Ĳ���
    Status history_req(OnlineClient * sender_client);

    void init_socket();                      // ��ʼ��server��socket������socket/bind/listen����
    void server_mainloop();                    // Server�������������ڱ������в����и������
};

#endif