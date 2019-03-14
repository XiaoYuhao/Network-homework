
#ifndef _CLIENT_OPERATING_

#define _CLIENT_OPERATING_

#include <vector>
#include <string>
#include <time.h>

#include "package.hpp"

const time_t TIMEOUT = 50; // Ĭ���룬��λ����

const int MAX_BUFFER_SIZE = 4096; // �������������

using std::string;

class OnlineClient
{
    /*
     * server�˴������������в�������
     * �����ͻ��˵������֡����������������ǰ״̬���շ������ݵ�
     * ��ÿ��server�˼�����һ���µĿͻ�������ʱ��������
     */
private:
    /*
     * ��������ͻ��˽���ʱ����ĸ�������
     */
    static int count; // ��ǰ�����Ķ������
    string client_ip; // �ͻ���ip
    time_t timer = 0; // ��ʱ��ʱ��

    char *read_buffer;    // ����������
    int read_buf_pointer; // ��������ָ��
    char *write_buffer;   //
    int write_buf_pointer;

public:
    int sock;             // ���ͻ��˵�������
    short userid = -1;     // �˿ͻ��˵�¼���û�-id
    bool dead = false;

private:
    /* 
     * �ͻ��˱�����Ҫ����Ϣ
     * ���������Ƿ�ɹ����ϴη���������ʱ���
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
    * Server�ඨʱ���ñ��������������������û��ļ�ʱ��
    * һ����ʱ����ֵ�����޶�����Ϊ��ʱ������false
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