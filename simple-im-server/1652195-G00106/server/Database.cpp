/*
 * Database.cpp
 * Database.hpp��ʵ���ļ�
 * 
 */

#include "Database.hpp"
#include "md5.h"

/*
 * �������뺯������Ϊ���ߺ���
 */
string encrypt(string password)
{
    MD5 md5(password);
    return md5.md5();
}


// =================================
// database���Ա����ʵ��
// =================================

/*
 * ���캯��
 * �����������ݿ�
 * TODO��������
 */
Database::Database()
{
    string log_file_name = "Databaselog_";
    string current_time = get_current_time();
    log_file_name += current_time;
    log_file_name += string(".log");
    log_stream.open(log_file_name.c_str());
    if(!log_stream.is_open()) {
        std::cerr << "DB log create error" << std::endl;
        exit(EXIT_FAILURE);
    }
    /* ��ʼ�� mysql ������ʧ�ܷ���NULL */
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        print_log("mysql_init failed");
    }

    /* �������ݿ⣬ʧ�ܷ���NULL
     1��mysqldû����
     2��û��ָ�����Ƶ����ݿ���� */
    if (mysql_real_connect(mysql, "localhost", db_username.c_str(), db_password.c_str(), db_name.c_str(), 0, NULL, 0) == NULL)
    {
        std::cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << std::endl;
        print_log("mysql_real_connect failed" );
    }

    /* �����ַ���������������ַ����룬��ʹ/etc/my.cnf������Ҳ���� */
    mysql_set_character_set(mysql, "gbk");
}

/*
 * ��¼����
 * �ṩ�û������룬�������ݿ�����
 * ���ݿⷵ�ط����û�����������û�����
 *      ����¼�ɹ����ж������Ƿ���Ч
 *               ��������Ч�������ݵ�ǰʱ����µ�¼ʱ�䣬����userid
 *               ��������Ч�����û���δ�������룬��ֵfirst_login������userid
 *      ����½ʧ�ܣ�����-1
 */
login_status Database::login(string username, string password, time_t current_time, short &userid)
{

    MYSQL_RES *result; // ��¼��ѯ�Ľ��
    MYSQL_ROW row;     // ����ѯ�Ľ������ȡ���Ļ���

    /* =================================== username =================================== */

    sprintf(query,
            "select count(*) "\
            "from %s "\
            "where username=\'%s\'",
            user_table_name.c_str(),
            username.c_str());

    if(send_query(query, &result) == ERROR)
    {
        print_log("login select query failed");
        mysql_free_result(result);
        return login_error;
    }

    // �Խ�����д���
    row = mysql_fetch_row(result);

    if (strcmp(row[0], "1")) // �û�������
    {
        print_log("username not found");
        mysql_free_result(result);
        return fail_username;
    }

    /* =================================== password =================================== */
    string encrypted_password = encrypt(password);

    sprintf(query,
            "select count(*) "\
            "from %s "\
            "where username=\'%s\' and password=\'%s\'",
            user_table_name.c_str(),
            username.c_str(),
            encrypted_password.c_str());

    if(send_query(query, &result) == ERROR)
    {
        print_log("login select query failed");
        mysql_free_result(result);
        return login_error;
    }

    // �Խ�����д���
    row = mysql_fetch_row(result);

    if (strcmp(row[0], "1")) // �û�����
    {
        print_log("password error");
        mysql_free_result(result);
        return fail_passwd;
    }

    print_log("username matches password!");
    /* =================================== userid =================================== */
        
    userid = get_userid(username);
    if(userid < 0)
    {
        print_log("login get userid query failed");
        mysql_free_result(result);
        return login_error;
    }
    /* =================================== valid =================================== */

    bool valid = password_valid(userid);
    if (valid)
    {
        print_log("update user login time to current time");
        sprintf(query, 
                "update %s "\
                "set logintime = FROM_UNIXTIME(%d) "\
                "where id = %d;",                   
                user_table_name.c_str(), 
                current_time, 
                userid);
        if(send_update(query) == ERROR)
        {
            print_log("login update query failed");
            mysql_free_result(result);
            return login_error;
        }
        mysql_free_result(result);
        return login_success;
    }
    else
    {
        print_log("user needs to update password");
        mysql_free_result(result);
        return first_login;
    }

    mysql_free_result(result);
    return login_error;
}

/* ͨ������userid���������룬���ҽ�passwordvalid��Ϊtrue */
int Database::update_password(short userid, string password)
{
    // cout<<password<<endl;
    string encrypted_password = encrypt(password);

    sprintf(query,
            "update %s "\
            "set password = \'%s\' "\
            "where id = %d;",
            user_table_name.c_str(), 
            encrypted_password.c_str(),
            userid);

    if(send_update(query) == ERROR)
    {
        print_log("update password query failed");
        return ERROR;
    }
    else
    {
        if (!password_valid(userid))
        {
            sprintf(query, 
                    "update %s "\
                    "set passwordvalid = 1 "\
                    "where id = %d;", 
                    user_table_name.c_str(), 
                    userid);

            if(send_update(query) == ERROR)
            {
                print_log("update password query failed");
                return ERROR;
            }
        }
    }
    // ���߶��ɹ�����1����һʧ�ܷ���-1
    return OK;
}

int Database::get_history(short id1, short id2, short ask_num, vector<history_reply_package*> &his_content)
{
    sprintf(query, 
            "select id1, id2, UNIX_TIMESTAMP(time), content "\
            "from %s "\
            "where (id1 = %d and id2 = %d) or (id2 = %d and id1 = %d) "\
            "order by time desc limit %d",
            history_table_name.c_str(),
            id1, 
            id2,
            id1, 
            id2,            
            ask_num);

    MYSQL_RES *result; // ��¼��ѯ�Ľ��
    MYSQL_ROW row;     // ����ѯ�Ľ������ȡ���Ļ���

    if(send_query(query, &result) == ERROR)
    {
        print_log("get_history query failed");
        return ERROR;
    }

    print_log("found history");

    short cnt = 0;
    while((row=mysql_fetch_row(result))) 
    {
        if(strlen(row[3]) > 1020)
        {
            print_log("Too long history");
            return ERROR;
        }

        history_reply_package *temp_pack = new history_reply_package(strlen(row[3])+1);

        temp_pack->sender_id = htons(atoi(row[0]));
        temp_pack->receiver_id = htons(atoi(row[1]));
        temp_pack->current_num = htons(cnt);
        temp_pack->chat_time = (atoi(row[2]));

        strcpy(temp_pack->data, row[3]);

        his_content.push_back(temp_pack);
        cnt++;
    }

    mysql_free_result(result);
    return OK;
}

int Database::add_history(short id1, short id2, time_t chattime, const char* content)
{
    sprintf(query, 
            "insert into %s "\
            "values( %hd, %hd, (FROM_UNIXTIME(%d)), \'%s\');", 
            history_table_name.c_str(),
            id1, 
            id2, 
            chattime, 
            content);


    if(send_update(query) == ERROR) {
        print_log("add_history query failed");
        return ERROR;
    }

    return OK;
}

int Database::get_config_information(int user_ID, config_information *conf_info)
{
    sprintf(query, 
            "select size, font, color, traceback "\
            "from %s "\
            "where id = %d;",
            setting_table_name.c_str(),
            user_ID);

    MYSQL_RES *result; // ��¼��ѯ�Ľ��
    MYSQL_ROW row;     // ����ѯ�Ľ������ȡ���Ļ���

    if(send_query(query, &result) == ERROR)
    {
        print_log("get_config query failed");
        return ERROR;
    }

    // �Խ�����д���
    row = mysql_fetch_row(result);
    if(row == NULL)
    {
        print_log("cannot find such config");
        mysql_free_result(result);
        return ERROR;
    }

    conf_info->font_size = htons(atoi(row[0]));
    conf_info->font_type = htons(atoi(row[1]));
    conf_info->font_color = htonl(atoi(row[2]));
    conf_info->history_list_size = htons(atoi(row[3]));
    
    print_log("load config successfully");

    mysql_free_result(result);
    return OK;
}

int Database::update_config_information(int user_ID, config_information conf_info)
{
    sprintf(query,
            "update %s "\
            "set size = %hd, font = %hd, color = %hd, traceback = %hd "\
            "where id = %d;",
            setting_table_name.c_str(), 
            conf_info.font_size, conf_info.font_type, conf_info.font_color, conf_info.history_list_size,
            user_ID);

    MYSQL_RES *result; // ��¼��ѯ�Ľ��
    MYSQL_ROW row;     // ����ѯ�Ľ������ȡ���Ļ���

    if(send_update(query) == ERROR)
    {
        print_log("update config query failed");
        return ERROR;
    }
    return OK;
}


int Database::get_all_users(vector<user_information> &usr_vec)
{
    // ������ݿ��еǼǵ������û�����Ϣ
    MYSQL_RES *result; // ��¼��ѯ�Ľ��
    MYSQL_ROW row;     // ����ѯ�Ľ������ȡ���Ļ���
    char query[40] = "select id, username from USER;";
    if(send_query(query, &result) == ERROR) {
        print_log("get_userid query failed");
        return ERROR;
    }
    // �Խ�����д���
    while((row = mysql_fetch_row(result))) {
        user_information user_info;
        short id = atoi(row[0]);
        char user_name[30];
        strcpy(user_name, row[1]);
        // cout << "id: " << id << " user_name: " << user_name << endl;
        user_info.user_ID = id;
        strcpy(user_info.user_name, row[1]);
        usr_vec.push_back(user_info);
    }
    return OK;
    
}

// ==================================== private ====================================

/* ˽�г�Ա������ͨ���û������userid */
short Database::get_userid(string username)
{
    sprintf(query, 
            "select id "\
            "from %s "\
            "where username = \'%s\';", 
            user_table_name.c_str(), 
            username.c_str());

    MYSQL_RES *result; // ��¼��ѯ�Ľ��
    MYSQL_ROW row;     // ����ѯ�Ľ������ȡ���Ļ���

    if(send_query(query, &result) == ERROR) {
        print_log("get_userid query failed");
        return ERROR;
    }
    // �Խ�����д���
    row = mysql_fetch_row(result);

    string res = row[0];
    short userid = atoi(res.c_str());

    mysql_free_result(result);
    return userid;
}

string Database::get_user_name(short id)
{
    // ͨ��user_id����û���
    MYSQL_RES *result; // ��¼��ѯ�Ľ��
    MYSQL_ROW row;     // ����ѯ�Ľ������ȡ���Ļ���
    char query_str[100];
    sprintf(query_str, "select username from USER where id = %hd;", id);
    if(send_query(query_str, &result) == ERROR) {
        print_log("get_user_name query failed");
        return string("#");
    }
    // �Խ�����д���
    row = mysql_fetch_row(result);
    string res = row[0];
    mysql_free_result(result);
    return res;
}



// �ж��û��Ƿ���¹����룬��δ���£�����0�����·���1
bool Database::password_valid(short userid)
{
    sprintf(query, 
            "select passwordvalid "\
            "from %s "\
            "where id = %d;", 
            user_table_name.c_str(), 
            userid);

    MYSQL_RES *result; // ��¼��ѯ�Ľ��
    MYSQL_ROW row;     // ����ѯ�Ľ������ȡ���Ļ���

    if(send_query(query, &result) == ERROR)
    {
        print_log("password_valid query failed");
        return false;
    }

    // �Խ�����д���
    row = mysql_fetch_row(result);
    string res = row[0];

    mysql_free_result(result);
    if ((res == "1") || (res == "true") || (res == "TRUE"))
        return true;
    else
        return false;
}

int Database::send_query(const char* query, MYSQL_RES **result)
{
    /* ���в�ѯ���ɹ�����0�����ɹ���0
     1����ѯ�ַ��������﷨����
     2����ѯ�����ڵ����ݱ� */
    print_log(query);
    if (mysql_query(mysql, query))
    {
        std::cout << "mysql_query failed(" << mysql_error(mysql) << ")" << std::endl;

        print_log( "mysql_query failed" );
        return ERROR;
    }

    /* ����ѯ����洢���������ִ����򷵻�NULL
     ע�⣺��ѯ���ΪNULL�����᷵��NULL */
    if ((*result = mysql_store_result(mysql)) == NULL)
    {
        print_log( "mysql_store_result failed");
        return ERROR;
    }

    return OK;
}

int Database::send_update(const char* query)
{
    /* ���в�ѯ���ɹ�����0�����ɹ���0
     1����ѯ�ַ��������﷨����
     2����ѯ�����ڵ����ݱ� */
    print_log(query);
    if (mysql_query(mysql, query))
    {
        std::cout << "mysql_query failed(" << mysql_error(mysql) << ")" << std::endl;

        print_log( "mysql_query failed" );
        return ERROR;
    }
    return OK;
}


// // /*=============================================================================*/
// // /*=============================================================================*/
// // /*=============================================================================*/
// // /*============ �˺������ã�demo.cpp�������selectʾ�����ڴ��������������� =============*/
// // /*=============================================================================*/
// // /*=============================================================================*/
using namespace std;
void Database::select_all()
{
    MYSQL_RES *result; // ��¼��ѯ�Ľ��
    MYSQL_ROW row;     // ����ѯ�Ľ������ȡ���Ļ���

    /* ���в�ѯ���ɹ�����0�����ɹ���0
     1����ѯ�ַ��������﷨����
     2����ѯ�����ڵ����ݱ� */
    if (mysql_query(mysql, "select * from USER")) {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
    }

    /* ����ѯ����洢���������ִ����򷵻�NULL
     ע�⣺��ѯ���ΪNULL�����᷵��NULL */
    if ((result = mysql_store_result(mysql))==NULL) {
        cout << "mysql_store_result failed" << endl;
    }

    /* ��ӡ��ǰ��ѯ���ļ�¼������ */
    cout << "select return " << (int)mysql_num_rows(result) << " records" << endl;
    /* ѭ����ȡ�������������ļ�¼
     1�����ص���˳����selectָ������˳����ͬ����row[0]��ʼ
     2���������ݿ�����ʲô���ͣ�C�ж��������ַ��������д�������б�Ҫ����Ҫ�Լ�����ת��
     3�������Լ�����Ҫ��֯�����ʽ */

    row=mysql_fetch_row(result);
    printf("%s\n", row[0] ? row[0] : "NULL" );
    printf("%s\n", row[1] ? row[1] : "NULL" );
    // while((row=mysql_fetch_row(result))) {
    //     cout << setiosflags(ios::left);             //��������
    //     cout << "id��" << setw(4) << row[0];     //���12λ�������
    //     cout << "username��" << setw(10)  << row[1];     //    8
    //     cout << "password��" << setw(10)  << row[2];     //    4
    //     cout << "regtime��" << setw(4)  << row[3];     //    4
    //     cout << "logintime��" << setw(4)  << row[4];     //    4
    //     cout << "passwordvalid��" << setw(4)  << row[5];     //    4
    //     cout << endl;
    // }
}
