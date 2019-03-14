#ifndef _PACKAGE_HEADER_

#define _PACKAGE_HEADER_

/*
 * package.hpp
 * 各种类型的数据包的定义
 * 注意：所有包都不需要自己初始化前8bit，由构造函数实现并将需要的位置转换为网络序
 * 数据包的内存已经在结构体生成时分配完成，且无需考虑内存释放问题
 * 
 */
#include <cstdlib>
#include <arpa/inet.h>

const char LOGIN_REQ            = 0x11;         // 客户端发来的请求登陆类型
const char LOGIN_REPLY          = 0x71;         // 服务器回复的登陆结果

const char ALIVE_CONFIRM        = 0x12;         // 客户端定期发送的正常连接包
const char AVAILABLE_CONFIRM    = 0x72;         // 服务器回复的正常连接包 （包括当前在线用户列表）

const char MSG_PACK             = 0x13;         // 客户端发送的数据包

const char FILE_REQ             = 0x16;          
const char FILE_PACK            = 0x76;         // 客户端发往服务器的文件包

const char FILE_REQ_TYPE        = 0x00;
const char FILE_AGREE_TYPE      = 0x01;
const char FILE_DISAGREE_TYPE   = 0x02;
const char FILE_STOP_TYPE       = 0x03;

const char CONFIG_PACKAGE       = 0x74;         // 服务器发往客户端的客户端配置数据
const char CONFIG_UPDATE        = 0x14;         // 客户端发往服务器的新的配置数据
const char QUIT_MESSAGE         = 0x15;         // 客户端发往服务器通知其本机退出
const char REPLACE_MESSAGE      = 0x80;         // 客户端发往服务器通知其本机退出


const char HISTORY_REQ          = 0x17;         // 客户端请求与某个对象的聊天记录
const char HISTORY_PACK         = 0x77;         // 服务器返回的聊天记录包
// const char DATA_SEND_REPLY      = 0x78;         // 服务器发送回数据发送方的确认包

const char LOGIN_SUCCESS        = 0x00;         // 服务器回复登陆请求类型为成功
const char LOGIN_FAIL_USERNAME  = 0x01;         // 服务器回复登陆请求类型为用户名错误
const char LOGIN_PASSWD         = 0x02;         // 服务器检测到本次登陆为第一次登陆，需要强制修改密码
const char LOGIN_FAIL_PASSWD    = 0x11;         // 服务器回复登陆请求类型为密码错误
const char LOGIN_REPLACE        = 0x10;         // 服务器回复登录成功且挤掉重复用户

const char LOGIN_TYPE           = 0x00;          // 客户端发来的登陆请求包
const char REGISTER_TYPE        = 0x01;          // 客户端发来的注册请求包
const char PASSWD_TYPE          = 0x02;          // 客户端请求修改密码

const int MAX_WORD_SIZE         = 1020;         // 

struct functional_package {
    // 所有包都包括的基本信息
    char package_type;          // 发送包的数据类型
    char functional_byte;       // 保留字，在某些包中会用到
    short package_length;       // 包的长度，根据类型初始化
};

struct config_information {
    // 客户端需要的所有配置信息，对应值参考报文格式文档
    short font_size_id;         // 字号的标志位
    short font_size;            // 字号的数据位
    short font_type_id;         // 字体的标志位
    short font_type;            // 字体的数据位
    short font_color_id;        // 文字颜色的标志位
    short zero_area;            // 颜色标志位后补2字节0
    int font_color;             // 文字颜色的数据位
    short history_list_size_id; // 历史记录条数的标志位
    short history_list_size;    // 历史记录条数的数据位
    int zero_line;              // 全零分隔行
    config_information() {
        font_size_id = htons(0x0001);
        font_type_id = htons(0x0002);
        font_color_id = htons(0x0003);
        history_list_size_id = htons(0x0004);
        zero_area = htons(0x0);
        zero_line = htons(0);
    }
};

struct user_information {
    short user_ID;
    short zero_area;
    char user_name[28];
    user_information() {
        zero_area = htons(0);
    }
};

struct login_ask_package {
    functional_package func_package;
    char username[28];
    char password[28];
    login_ask_package() {
        func_package.package_type = LOGIN_REQ;
        func_package.functional_byte = LOGIN_TYPE;
        func_package.package_length = htons(0x003C);
    }
};

struct register_ask_package {
    functional_package func_package;
    char username[28];
    char password[28];
    register_ask_package() {
        func_package.package_type = LOGIN_REQ;
        func_package.functional_byte = REGISTER_TYPE;
        func_package.package_length = htons(0x003C);
    }
};

struct update_passwd_package {
    functional_package func_package;
    char username[28];
    char password[28];
    update_passwd_package() {
        func_package.package_type = LOGIN_REQ;
        func_package.functional_byte = PASSWD_TYPE;
        func_package.package_length = htons(0x003C);
    }
};

struct login_reply_package {
    // 需要手动填写回复结果和用户id
    functional_package func_package;
    short user_ID;
    short zero_area;
    login_reply_package() {
        func_package.package_type = LOGIN_REPLY;
        func_package.package_length = htons(0x0008);
        zero_area = htons(0);
    }
};

struct alive_confirm_package {
    // 不需要任何初始化操作，实例化后直接操作即可
    functional_package func_package;
    alive_confirm_package() {
        func_package.package_type = ALIVE_CONFIRM;
        func_package.functional_byte = 0x0;
        func_package.package_length = htons(0x0004);
    }
};

struct available_confirm_package {
    functional_package func_package;
    short usernum;
    short zero;
    short *online_user;
    available_confirm_package(short len) {
        func_package.package_type = AVAILABLE_CONFIRM;
        func_package.functional_byte = 0x0;

        if(len % 2 != 0)
            len++;

        online_user=new short[len];
        func_package.package_length = htons(sizeof(short)*2 + len * sizeof(short) + sizeof(functional_package));

        // 后续实现聊天记录数据结构后再确定该包的长度
    }
    ~available_confirm_package(){
        delete[] online_user;
        online_user=NULL;
    }
};

struct msg_package 
{
    // 需要手动设置发送/接收端用户id、拷贝数据段data
    functional_package func_package;
    short sender_id;        // 发送端用户id
    short receiver_id;      // 接收端用户id
    char data[1020];
    msg_package() 
    {
        func_package.package_type = MSG_PACK;
        func_package.functional_byte = 0x0;
        func_package.package_length = htons(1028);
    }
};

struct filereq_package 
{
    // 需要手动设置发送/接收端用户id、拷贝数据段data
    functional_package func_package;
    short sender_id;        // 发送端用户id
    short receiver_id;      // 接收端用户id
    char file_name[32];
    int file_pack_num;
    int file_size;
    filereq_package(char TYPE) 
    {
        func_package.package_type = FILE_REQ;
        func_package.functional_byte = TYPE;
        func_package.package_length = htons(48);
    }
};

struct file_package 
{
    // 需要手动设置发送/接收端用户id、拷贝数据段data
    functional_package func_package;
    short sender_id;        // 发送端用户id
    short receiver_id;      // 接收端用户id
    char file_name[32];
    int file_pack_num;
    int current_pack_num;
    char pack_content[1024];
    file_package(int len) 
    {
        func_package.package_type = FILE_PACK;
        func_package.functional_byte = 0x0;
        func_package.package_length = htons(len);
    }
};

struct config_package {
    // 服务器在客户端登录时发送的配置信息包
    functional_package func_package;
    config_information config_info;
    short user_count;       // 服务器当前的用户总数
    short zero_area;
    user_information *users; // 用户
    config_package(short count) {
        func_package.package_type = CONFIG_PACKAGE;
        func_package.functional_byte = 0x0;
        user_count = htons(count);
        zero_area = htons(0);
        users = new user_information[count];
        // 初始化长度，随着用户数增加需要继续增加长度
        func_package.package_length = htons(sizeof(functional_package) + sizeof(config_information) 
        + 2 * sizeof(short) + count * sizeof(user_information));
    }
    ~config_package() {
        delete[] users;
        users = NULL;
    }
};

struct config_update_package {
    // 未完成
    functional_package func_package;
    config_update_package() {
        func_package.package_type = CONFIG_UPDATE;
        func_package.functional_byte = 0x0;
    }
};

struct quit_message_package {
    functional_package func_package;
    quit_message_package() {
        func_package.package_type = QUIT_MESSAGE;
        func_package.functional_byte = 0x0;
        func_package.package_length = htons(0x0004);
    }
};

struct replace_package {
    functional_package func_package;
    replace_package() {
        func_package.package_type = REPLACE_MESSAGE;
        func_package.functional_byte = 0x0;
        func_package.package_length = htons(0x0004);
    }
};

struct history_ask_package {
    functional_package func_package;
    short sender_id;        // 发送端用户id
    short receiver_id;      // 接收端用户id
    short history_ask_num;
    short zero_area;
    history_ask_package() {
        func_package.package_type = HISTORY_REQ;
        func_package.functional_byte = 0x0;
        func_package.package_length = htons(sizeof(functional_package) + 4 * sizeof(short));    
        zero_area = htons(0);
    }
};

struct history_reply_package 
{
    functional_package func_package;
    short sender_id;        // 发送端用户id
    short receiver_id;      // 接收端用户id
    short total_num;
    short current_num;
    int chat_time;
    char *data;
    history_reply_package(int len) 
    {
        data = new char[len];
        func_package.package_type = HISTORY_PACK;
        func_package.functional_byte = 0x0;
        func_package.package_length = htons(len + sizeof(functional_package) + 4 * sizeof(short) + sizeof(int));    
    }
    ~history_reply_package()
    {
        delete[] data;
        data = NULL;
    } 
};


#endif