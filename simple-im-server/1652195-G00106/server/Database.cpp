/*
 * Database.cpp
 * Database.hpp的实现文件
 * 
 */

#include "Database.hpp"
#include "md5.h"

/*
 * 加密密码函数，作为工具函数
 */
string encrypt(string password)
{
    MD5 md5(password);
    return md5.md5();
}


// =================================
// database类成员函数实现
// =================================

/*
 * 构造函数
 * 负责连接数据库
 * TODO：出错处理
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
    /* 初始化 mysql 变量，失败返回NULL */
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        print_log("mysql_init failed");
    }

    /* 连接数据库，失败返回NULL
     1、mysqld没运行
     2、没有指定名称的数据库存在 */
    if (mysql_real_connect(mysql, "localhost", db_username.c_str(), db_password.c_str(), db_name.c_str(), 0, NULL, 0) == NULL)
    {
        std::cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << std::endl;
        print_log("mysql_real_connect failed" );
    }

    /* 设置字符集，否则读出的字符乱码，即使/etc/my.cnf中设置也不行 */
    mysql_set_character_set(mysql, "gbk");
}

/*
 * 登录过程
 * 提供用户名密码，进入数据库搜索
 * 数据库返回符合用户名和密码的用户数量
 *      若登录成功，判断密码是否有效
 *               若密码有效，将根据当前时间更新登录时间，返回userid
 *               若密码无效，即用户尚未更新密码，赋值first_login并返回userid
 *      若登陆失败，返回-1
 */
login_status Database::login(string username, string password, time_t current_time, short &userid)
{

    MYSQL_RES *result; // 记录查询的结果
    MYSQL_ROW row;     // 将查询的结果逐行取出的缓存

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

    // 对结果进行处理
    row = mysql_fetch_row(result);

    if (strcmp(row[0], "1")) // 用户不存在
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

    // 对结果进行处理
    row = mysql_fetch_row(result);

    if (strcmp(row[0], "1")) // 用户存在
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

/* 通过给定userid，更新密码，并且将passwordvalid置为true */
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
    // 两者都成功返回1，任一失败返回-1
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

    MYSQL_RES *result; // 记录查询的结果
    MYSQL_ROW row;     // 将查询的结果逐行取出的缓存

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

    MYSQL_RES *result; // 记录查询的结果
    MYSQL_ROW row;     // 将查询的结果逐行取出的缓存

    if(send_query(query, &result) == ERROR)
    {
        print_log("get_config query failed");
        return ERROR;
    }

    // 对结果进行处理
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

    MYSQL_RES *result; // 记录查询的结果
    MYSQL_ROW row;     // 将查询的结果逐行取出的缓存

    if(send_update(query) == ERROR)
    {
        print_log("update config query failed");
        return ERROR;
    }
    return OK;
}


int Database::get_all_users(vector<user_information> &usr_vec)
{
    // 获得数据库中登记的所有用户的信息
    MYSQL_RES *result; // 记录查询的结果
    MYSQL_ROW row;     // 将查询的结果逐行取出的缓存
    char query[40] = "select id, username from USER;";
    if(send_query(query, &result) == ERROR) {
        print_log("get_userid query failed");
        return ERROR;
    }
    // 对结果进行处理
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

/* 私有成员函数，通过用户名获得userid */
short Database::get_userid(string username)
{
    sprintf(query, 
            "select id "\
            "from %s "\
            "where username = \'%s\';", 
            user_table_name.c_str(), 
            username.c_str());

    MYSQL_RES *result; // 记录查询的结果
    MYSQL_ROW row;     // 将查询的结果逐行取出的缓存

    if(send_query(query, &result) == ERROR) {
        print_log("get_userid query failed");
        return ERROR;
    }
    // 对结果进行处理
    row = mysql_fetch_row(result);

    string res = row[0];
    short userid = atoi(res.c_str());

    mysql_free_result(result);
    return userid;
}

string Database::get_user_name(short id)
{
    // 通过user_id获得用户名
    MYSQL_RES *result; // 记录查询的结果
    MYSQL_ROW row;     // 将查询的结果逐行取出的缓存
    char query_str[100];
    sprintf(query_str, "select username from USER where id = %hd;", id);
    if(send_query(query_str, &result) == ERROR) {
        print_log("get_user_name query failed");
        return string("#");
    }
    // 对结果进行处理
    row = mysql_fetch_row(result);
    string res = row[0];
    mysql_free_result(result);
    return res;
}



// 判断用户是否更新过密码，若未更新，返回0，更新返回1
bool Database::password_valid(short userid)
{
    sprintf(query, 
            "select passwordvalid "\
            "from %s "\
            "where id = %d;", 
            user_table_name.c_str(), 
            userid);

    MYSQL_RES *result; // 记录查询的结果
    MYSQL_ROW row;     // 将查询的结果逐行取出的缓存

    if(send_query(query, &result) == ERROR)
    {
        print_log("password_valid query failed");
        return false;
    }

    // 对结果进行处理
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
    /* 进行查询，成功返回0，不成功非0
     1、查询字符串存在语法错误
     2、查询不存在的数据表 */
    print_log(query);
    if (mysql_query(mysql, query))
    {
        std::cout << "mysql_query failed(" << mysql_error(mysql) << ")" << std::endl;

        print_log( "mysql_query failed" );
        return ERROR;
    }

    /* 将查询结果存储起来，出现错误则返回NULL
     注意：查询结果为NULL，不会返回NULL */
    if ((*result = mysql_store_result(mysql)) == NULL)
    {
        print_log( "mysql_store_result failed");
        return ERROR;
    }

    return OK;
}

int Database::send_update(const char* query)
{
    /* 进行查询，成功返回0，不成功非0
     1、查询字符串存在语法错误
     2、查询不存在的数据表 */
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
// // /*============ 此函数无用，demo.cpp里给出的select示例，在此做备份以留套用 =============*/
// // /*=============================================================================*/
// // /*=============================================================================*/
using namespace std;
void Database::select_all()
{
    MYSQL_RES *result; // 记录查询的结果
    MYSQL_ROW row;     // 将查询的结果逐行取出的缓存

    /* 进行查询，成功返回0，不成功非0
     1、查询字符串存在语法错误
     2、查询不存在的数据表 */
    if (mysql_query(mysql, "select * from USER")) {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
    }

    /* 将查询结果存储起来，出现错误则返回NULL
     注意：查询结果为NULL，不会返回NULL */
    if ((result = mysql_store_result(mysql))==NULL) {
        cout << "mysql_store_result failed" << endl;
    }

    /* 打印当前查询到的记录的数量 */
    cout << "select return " << (int)mysql_num_rows(result) << " records" << endl;
    /* 循环读取所有满足条件的记录
     1、返回的列顺序与select指定的列顺序相同，从row[0]开始
     2、不论数据库中是什么类型，C中都当作是字符串来进行处理，如果有必要，需要自己进行转换
     3、根据自己的需要组织输出格式 */

    row=mysql_fetch_row(result);
    printf("%s\n", row[0] ? row[0] : "NULL" );
    printf("%s\n", row[1] ? row[1] : "NULL" );
    // while((row=mysql_fetch_row(result))) {
    //     cout << setiosflags(ios::left);             //输出左对齐
    //     cout << "id：" << setw(4) << row[0];     //宽度12位，左对齐
    //     cout << "username：" << setw(10)  << row[1];     //    8
    //     cout << "password：" << setw(10)  << row[2];     //    4
    //     cout << "regtime：" << setw(4)  << row[3];     //    4
    //     cout << "logintime：" << setw(4)  << row[4];     //    4
    //     cout << "passwordvalid：" << setw(4)  << row[5];     //    4
    //     cout << endl;
    // }
}
