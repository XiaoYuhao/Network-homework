#ifndef _PACKAGE_HEADER_

#define _PACKAGE_HEADER_

/*
 * package.hpp
 * �������͵����ݰ��Ķ���
 * ע�⣺���а�������Ҫ�Լ���ʼ��ǰ8bit���ɹ��캯��ʵ�ֲ�����Ҫ��λ��ת��Ϊ������
 * ���ݰ����ڴ��Ѿ��ڽṹ������ʱ������ɣ������迼���ڴ��ͷ�����
 * 
 */
#include <cstdlib>
#include <arpa/inet.h>

const char LOGIN_REQ            = 0x11;         // �ͻ��˷����������½����
const char LOGIN_REPLY          = 0x71;         // �������ظ��ĵ�½���

const char ALIVE_CONFIRM        = 0x12;         // �ͻ��˶��ڷ��͵��������Ӱ�
const char AVAILABLE_CONFIRM    = 0x72;         // �������ظ����������Ӱ� ��������ǰ�����û��б�

const char MSG_PACK             = 0x13;         // �ͻ��˷��͵����ݰ�

const char FILE_REQ             = 0x16;          
const char FILE_PACK            = 0x76;         // �ͻ��˷������������ļ���

const char FILE_REQ_TYPE        = 0x00;
const char FILE_AGREE_TYPE      = 0x01;
const char FILE_DISAGREE_TYPE   = 0x02;
const char FILE_STOP_TYPE       = 0x03;

const char CONFIG_PACKAGE       = 0x74;         // �����������ͻ��˵Ŀͻ�����������
const char CONFIG_UPDATE        = 0x14;         // �ͻ��˷������������µ���������
const char QUIT_MESSAGE         = 0x15;         // �ͻ��˷���������֪ͨ�䱾���˳�
const char REPLACE_MESSAGE      = 0x80;         // �ͻ��˷���������֪ͨ�䱾���˳�


const char HISTORY_REQ          = 0x17;         // �ͻ���������ĳ������������¼
const char HISTORY_PACK         = 0x77;         // ���������ص������¼��
// const char DATA_SEND_REPLY      = 0x78;         // ���������ͻ����ݷ��ͷ���ȷ�ϰ�

const char LOGIN_SUCCESS        = 0x00;         // �������ظ���½��������Ϊ�ɹ�
const char LOGIN_FAIL_USERNAME  = 0x01;         // �������ظ���½��������Ϊ�û�������
const char LOGIN_PASSWD         = 0x02;         // ��������⵽���ε�½Ϊ��һ�ε�½����Ҫǿ���޸�����
const char LOGIN_FAIL_PASSWD    = 0x11;         // �������ظ���½��������Ϊ�������
const char LOGIN_REPLACE        = 0x10;         // �������ظ���¼�ɹ��Ҽ����ظ��û�

const char LOGIN_TYPE           = 0x00;          // �ͻ��˷����ĵ�½�����
const char REGISTER_TYPE        = 0x01;          // �ͻ��˷�����ע�������
const char PASSWD_TYPE          = 0x02;          // �ͻ��������޸�����

const int MAX_WORD_SIZE         = 1020;         // 

struct functional_package {
    // ���а��������Ļ�����Ϣ
    char package_type;          // ���Ͱ�����������
    char functional_byte;       // �����֣���ĳЩ���л��õ�
    short package_length;       // ���ĳ��ȣ��������ͳ�ʼ��
};

struct config_information {
    // �ͻ�����Ҫ������������Ϣ����Ӧֵ�ο����ĸ�ʽ�ĵ�
    short font_size_id;         // �ֺŵı�־λ
    short font_size;            // �ֺŵ�����λ
    short font_type_id;         // ����ı�־λ
    short font_type;            // ���������λ
    short font_color_id;        // ������ɫ�ı�־λ
    short zero_area;            // ��ɫ��־λ��2�ֽ�0
    int font_color;             // ������ɫ������λ
    short history_list_size_id; // ��ʷ��¼�����ı�־λ
    short history_list_size;    // ��ʷ��¼����������λ
    int zero_line;              // ȫ��ָ���
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
    // ��Ҫ�ֶ���д�ظ�������û�id
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
    // ����Ҫ�κγ�ʼ��������ʵ������ֱ�Ӳ�������
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

        // ����ʵ�������¼���ݽṹ����ȷ���ð��ĳ���
    }
    ~available_confirm_package(){
        delete[] online_user;
        online_user=NULL;
    }
};

struct msg_package 
{
    // ��Ҫ�ֶ����÷���/���ն��û�id���������ݶ�data
    functional_package func_package;
    short sender_id;        // ���Ͷ��û�id
    short receiver_id;      // ���ն��û�id
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
    // ��Ҫ�ֶ����÷���/���ն��û�id���������ݶ�data
    functional_package func_package;
    short sender_id;        // ���Ͷ��û�id
    short receiver_id;      // ���ն��û�id
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
    // ��Ҫ�ֶ����÷���/���ն��û�id���������ݶ�data
    functional_package func_package;
    short sender_id;        // ���Ͷ��û�id
    short receiver_id;      // ���ն��û�id
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
    // �������ڿͻ��˵�¼ʱ���͵�������Ϣ��
    functional_package func_package;
    config_information config_info;
    short user_count;       // ��������ǰ���û�����
    short zero_area;
    user_information *users; // �û�
    config_package(short count) {
        func_package.package_type = CONFIG_PACKAGE;
        func_package.functional_byte = 0x0;
        user_count = htons(count);
        zero_area = htons(0);
        users = new user_information[count];
        // ��ʼ�����ȣ������û���������Ҫ�������ӳ���
        func_package.package_length = htons(sizeof(functional_package) + sizeof(config_information) 
        + 2 * sizeof(short) + count * sizeof(user_information));
    }
    ~config_package() {
        delete[] users;
        users = NULL;
    }
};

struct config_update_package {
    // δ���
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
    short sender_id;        // ���Ͷ��û�id
    short receiver_id;      // ���ն��û�id
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
    short sender_id;        // ���Ͷ��û�id
    short receiver_id;      // ���ն��û�id
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