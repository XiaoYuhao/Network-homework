/*
 * 用来与数据库进行交互的文件
 * 提供各种交互接口
 * 数据库的实际连接封装在本文件中
 * 只需在整个程序中建立一个对象即可
 */

#ifndef _DATABASE_HEADER_
#define _DATABASE_HEADER_

#include <iostream> // cin,cout等
#include <iomanip>  // setw等
#include <string>
#include <mysql.h>  // mysql特有
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <vector>
#include "package.hpp"
#include "tools.hpp"

#define ERROR -1
#define OK 1
using std::string;
using std::cout;
using std::endl;
using std::ofstream;
using std::vector;

const string db_username = "u1652195";       // 数据库登录用户名
const string db_password = "u1652195";    // 数据库登录密码
const string db_name = "db1652195";           // 数据库名称


// 三个数据表的名称
const string user_table_name = "USER";
const string history_table_name = "HISTORY";
const string setting_table_name = "CONFIG";

typedef int Status;

enum login_status { login_success = 1, first_login = 2, fail_username = -1, fail_passwd = -2, login_error = 0};


/*
 * 需要建立的数据库
 * (USER) - id - username - password - regtime - logintime - passwordvalid
 * (HISTORY) - id - id2 - time - content
 * (SETTING) - id - color - TrackBackNum
 */
class Database {

private:
    MYSQL     *mysql;                               // mysql
    char query[2048];
    char temp_log[100];
    ofstream log_stream;

public:
    Database();

    ~Database()
    {
        /* 关闭整个连接 */
        mysql_close(mysql);
    }
    login_status login(string username, string password, time_t current_time, short &userid);// 按用户名和密码登录，并更新登录时间
    int update_password(short userid, string password);                  // 按userid更新密码
   
    int get_history(short id1, short id2, short ask_num, vector<history_reply_package*> &his_content);
    int add_history(short id1, short id2, time_t chattime, const char *content);
    string get_user_name(short id);
    int get_all_users(vector<user_information> &usr_vec);
    int get_config_information(int user_ID, config_information *conf_info);
    int update_config_information(int user_ID, config_information conf_info);

    void select_all();
    //    bool reg(string username, string password);     // 按用户名和密码注册
    
private:
    int send_query(const char* query, MYSQL_RES **result);
    int send_update(const char* query);

    bool password_valid(short userid);            // 按userid判断是否初次登录
    short get_userid(string username);            //通过用户名获得userid
    
    void print_log(string content)
    {
        log_stream << "time: " << get_current_time() << ": "
                   << content << std::endl;
    }
};



#endif