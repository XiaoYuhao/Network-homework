/*
 * ���������ݿ���н������ļ�
 * �ṩ���ֽ����ӿ�
 * ���ݿ��ʵ�����ӷ�װ�ڱ��ļ���
 * ֻ�������������н���һ�����󼴿�
 */

#ifndef _DATABASE_HEADER_
#define _DATABASE_HEADER_

#include <iostream> // cin,cout��
#include <iomanip>  // setw��
#include <string>
#include <mysql.h>  // mysql����
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

const string db_username = "u1652195";       // ���ݿ��¼�û���
const string db_password = "u1652195";    // ���ݿ��¼����
const string db_name = "db1652195";           // ���ݿ�����


// �������ݱ������
const string user_table_name = "USER";
const string history_table_name = "HISTORY";
const string setting_table_name = "CONFIG";

typedef int Status;

enum login_status { login_success = 1, first_login = 2, fail_username = -1, fail_passwd = -2, login_error = 0};


/*
 * ��Ҫ���������ݿ�
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
        /* �ر��������� */
        mysql_close(mysql);
    }
    login_status login(string username, string password, time_t current_time, short &userid);// ���û����������¼�������µ�¼ʱ��
    int update_password(short userid, string password);                  // ��userid��������
   
    int get_history(short id1, short id2, short ask_num, vector<history_reply_package*> &his_content);
    int add_history(short id1, short id2, time_t chattime, const char *content);
    string get_user_name(short id);
    int get_all_users(vector<user_information> &usr_vec);
    int get_config_information(int user_ID, config_information *conf_info);
    int update_config_information(int user_ID, config_information conf_info);

    void select_all();
    //    bool reg(string username, string password);     // ���û���������ע��
    
private:
    int send_query(const char* query, MYSQL_RES **result);
    int send_update(const char* query);

    bool password_valid(short userid);            // ��userid�ж��Ƿ���ε�¼
    short get_userid(string username);            //ͨ���û������userid
    
    void print_log(string content)
    {
        log_stream << "time: " << get_current_time() << ": "
                   << content << std::endl;
    }
};



#endif