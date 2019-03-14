#include "Server.hpp"

Server::Server(int port)
{
    server_port = port;
    string log_file_name = "Serverlog_";
    string current_time = get_current_time();
    log_file_name += current_time;
    log_file_name += string(".log");

    log_file_stream.open(log_file_name.c_str());
    if (!log_file_stream.is_open())
    {
        cerr << "server log create error" << endl;
        exit(EXIT_FAILURE);
    }
    print_log("server started");
    print_hint("server started");
}

/*
 * Server����ڱ���socket�����ĳ�ʼ��
 * sock_fd����Ϊ������ģʽ
 */
void Server::init_socket()
{
    struct sockaddr_in my_addr; // ������������ַ��Ϣ
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(server_port);

    // ��ʼ��socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        print_error_log("socket error");
        exit(EXIT_FAILURE);
    }

    // ����socket������
    int flags;
    flags = fcntl(server_sock, F_GETFL, 0);
    if (flags < 0)
    {
        print_error_log("fcntl(F_GETFL) error");
        exit(EXIT_FAILURE);
    }
    if (fcntl(server_sock, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        print_error_log("fcntl(F_SETFL) error");
        exit(EXIT_FAILURE);
    }

    long flag = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));
    if (bind(server_sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        print_error_log("bind error");
        exit(EXIT_FAILURE);
    }
    if (listen(server_sock, QUEUE_SIZE) < 0)
    {
        print_error_log("listen error");
        exit(EXIT_FAILURE);
    }
    print_log("init - ������socket�������");
}

Status Server::login_req(OnlineClient *sender_client, char func_byte)
{
    // �ͻ��˷��͵�¼���󣬱�ʾ��ʱ�������µ�¼
    int login_res;
    if (func_byte == PASSWD_TYPE)
    {
        login_res = change_password(sender_client);
    }
    else
    {
        login_res = user_login(sender_client);
        if (login_res == OK)
        {
            print_log("login - �ͻ��˵�¼��ɣ�׼������������Ϣ��");

            int config_res = send_config(sender_client);
            if (config_res == OK)
            {
                print_log("login - ������Ϣ�����ͳɹ�");
            }
            else
            {
                print_log("login - ������Ϣ������ʧ��");
            }
        }
        else
        {
            print_log("login - ��ǰ�ͻ��˵�¼ʧ��");
            return ERROR;
        }
    }
    return OK;
}

Status Server::user_login(OnlineClient *sender_client)
{
    // ����ͻ��˽������Ӻ�ĵ�¼����

    print_log("login - �յ���¼����");
    login_ask_package login_pack;

    /* =================================== process request =================================== */
    int res;
    res = recv(sender_client->sock, &login_pack, sizeof(login_ask_package), 0);
    if (res <= 0)
    {
        print_error_log("login - recv error");
    }

    // login_pack.func_package.package_length = ntohs(login_pack.func_package.package_length);

    // �ͻ��˷����������ǵ�½���󣬴����½����
    string username = login_pack.username;
    string password = login_pack.password;

    time_t current_time = time(0);

    short temp_id;
    login_status login_ret = database.login(username, password, current_time, temp_id);

    /* =================================== send reply =================================== */
    login_reply_package reply_pack;
    if (login_ret > 0)
    {
        if (id_map_client.count(temp_id) > 0)
        {
            OnlineClient *dead_client = id_map_client[temp_id];
            replace_package rep_pack;
            send(dead_client->sock, &rep_pack, sizeof(replace_package), 0);
            dead_client->dead = true;

            sprintf(temp_log, "quit client - userid = %d �ظ���¼����", dead_client->userid);
            print_log(temp_log);
            print_hint(temp_log);

            reply_pack.func_package.functional_byte = LOGIN_REPLACE;
            print_log("login - ��ǰ�û���¼�ɹ��Ҽ�����һ�ͻ����ϵĵ�¼");
        }
        else
        {
            if (login_ret == login_success) //successful
            {
                // ��½�ɹ�
                reply_pack.func_package.functional_byte = LOGIN_SUCCESS;
                print_log("login - ��ǰ�û���¼�ɹ��Ҳ���Ҫ�޸�����");
            }
            else if (login_ret == first_login)
            {
                // ���û���һ�ε�½
                reply_pack.func_package.functional_byte = LOGIN_PASSWD;
                print_log("login - ���û��ѵ�¼����Ҫ�޸�����");
            }
        }

        sender_client->userid = temp_id;
        id_map_client.insert(make_pair(temp_id, sender_client));

        reply_pack.user_ID = htons(temp_id);
        int send_res;
        do
        {
            send_res = send(sender_client->sock, &reply_pack, sizeof(login_reply_package), 0);
            if (send_res <= 0)
            {
                print_error_log("login - send reply pack error");
            }
        } while (send_res <= 0);

        print_log("login - ���͵�¼ȷ�ϰ����");
        return OK;
    }
    else
    {
        if (login_ret == login_error)
        {
            print_log("login - ��¼ʧ�ܣ����ݿ��ѯ����");
            return ERROR;
        }
        else if (login_ret == fail_username)
        {
            // ��½���Ϸ�
            reply_pack.func_package.functional_byte = LOGIN_FAIL_USERNAME;
            print_log("login - ��ǰ�û��û������Ϸ�");
        }
        else if (login_ret == fail_passwd)
        {
            reply_pack.func_package.functional_byte = LOGIN_FAIL_PASSWD;
            print_log("login - ��ǰ�û����벻�Ϸ�");
        }

        reply_pack.user_ID = htons(0);
        int send_res;
        do
        {
            send_res = send(sender_client->sock, &reply_pack, sizeof(login_reply_package), 0);
            if (send_res <= 0)
            {
                print_error_log("login - send reply pack error");
            }
        } while (send_res <= 0);
        print_log("login - ���͵�¼ȷ�ϰ����");
        return ERROR;
    }
}

Status Server::change_password(OnlineClient *sender_client)
{
    // �ͻ��������޸�����
    // �յ��ͻ��˷������޸������������Ϊ�ÿͻ����Ѿ��˳���¼���������б����Ƴ�
    short sender_id = sender_client->userid;
    sprintf(temp_log, "change passwd - userid = %hd�����޸���������", sender_id);
    print_log(temp_log);

    update_passwd_package passwd_pack;
    int recv_res = recv(sender_client->sock, &passwd_pack, sizeof(update_passwd_package), 0);
    if (recv_res <= 0)
    {
        sprintf(temp_log, "change passwd - ��idΪ%hd�Ŀͻ��˽��մ���", sender_id);
        print_error_log(temp_log);
        return ERROR;
    }

    // cout<<"length = "<<ntohs(passwd_pack.func_package.package_length)<<endl;
    // cout<<"username: "<<passwd_pack.username<<endl;

    string new_passwd = passwd_pack.password;
    // cout<<"newpassword:"<<passwd_pack.password<<endl;

    login_reply_package update_reply_package;
    update_reply_package.user_ID = htons(sender_id);

    if (database.update_password(sender_id, new_passwd) == OK)
    {
        update_reply_package.func_package.functional_byte = LOGIN_SUCCESS;
        sprintf(temp_log, "change passwd - idΪ%hd�Ŀͻ����޸�����ɹ�", sender_id);
        print_log(temp_log);
    }
    else
    {
        update_reply_package.func_package.functional_byte = LOGIN_FAIL_PASSWD;
        sprintf(temp_log, "change passwd - idΪ%hd�Ŀͻ����޸�����ʧ��", sender_id);
        print_log(temp_log);
    }

    int send_res = send(sender_client->sock, &update_reply_package, sizeof(login_reply_package), 0);
    if (send_res <= 0)
    {
        sprintf(temp_log, "change passwd - ��idΪ%hd�Ŀͻ��˷��ʹ���", sender_id);
        print_error_log(temp_log);
        return ERROR;
    }
    return OK;
}

Status Server::send_config(OnlineClient * sender_client)
{
    // ����������Ϣ��
    // ���Ȳ�ѯ���ݿ��ȡ�����û���Ϣ
    int ret_value;
    vector<user_information> user_info_vec;

    int query_res = database.get_all_users(user_info_vec);
    if (query_res < 0)
    {
        print_log("config - ���ݿ��ѯget_all_users����");
        return ERROR;
    }
    int user_count = user_info_vec.size();

    config_package conf_pack(user_count);

    // ��ѯ��Ӧ�û���������Ϣ
    int user_id = sock_map_client[sender_client->sock]->userid;
    query_res = database.get_config_information(user_id, &conf_pack.config_info);
    if (query_res < 0)
    {
        print_log("config - ���ݿ��ѯget_config_information����");
        return ERROR;
    }

    // �����ֽ�����buf������������
    // �Ա������ָ���еķ����ڴ�ռ䷢�ʹ��������
    int buf_size = user_count * sizeof(user_information) + sizeof(functional_package) + sizeof(config_information) + 2 * sizeof(short);
    char *buf = new char[buf_size];
    int pointer = 0; // buf��ָ��

    memcpy(buf, &conf_pack.func_package, sizeof(functional_package));
    pointer += sizeof(functional_package);

    memcpy(&buf[pointer], &conf_pack.config_info, sizeof(config_information));
    pointer += sizeof(config_information);

    memcpy(&buf[pointer], &conf_pack.user_count, sizeof(short));
    pointer += sizeof(short);

    memcpy(&buf[pointer], &conf_pack.zero_area, sizeof(short));
    pointer += sizeof(short);

    for (int i = 0; i < user_count; i++)
    {
        // �������user_information����д��buf������
        user_information user_info = user_info_vec[i];
        // ��id��Ϣת������
        user_info.user_ID = htons(user_info.user_ID);

        memcpy(&buf[pointer], &user_info, sizeof(user_information));
        pointer += sizeof(user_information);
    }

    if (pointer == buf_size)
    {
        print_log("config - ������Ϣ���Ѿ�׼����ɣ�׼�����͸��ͻ���");

        int res = send(sender_client->sock, buf, buf_size, 0);
        if (res == buf_size)
        {
            print_log("config - ������Ϣ���������");
            ret_value = OK;
        }
        else
        {
            print_log("config - ������Ϣ������ʧ��");
            ret_value = ERROR;
        }
        // }
    }
    else
    {
        print_log("config - д��buf��������з�������");
        ret_value = ERROR;
    }
    delete[] buf;
    return ret_value;
}

void Server::quit_client(OnlineClient *client_to_delete)
{
    sprintf(temp_log, "quit - �ͻ��� (sock = %d) �˳�.", client_to_delete->sock);
    print_log(temp_log);

    close(client_to_delete->sock);

    auto it = find(online_clients.begin(), online_clients.end(), client_to_delete);
    if (it != online_clients.end())
        online_clients.erase(it);

    if (sock_map_client.count(client_to_delete->sock) > 0)
        sock_map_client.erase(client_to_delete->sock);

    if (id_map_client.count(client_to_delete->userid) > 0)
        id_map_client.erase(client_to_delete->userid);

    // �ͷ�client���ڴ�
    delete client_to_delete;
}

Status Server::accept_new_client(fd_set *p_rfd, int &maxfd)
{
    int new_sock;
    struct sockaddr_in remote_addr;
    int sin_size = sizeof(struct sockaddr_in);
    if ((new_sock = accept(server_sock, (struct sockaddr *)&remote_addr, (socklen_t *)&sin_size)) < 0)
    {
        print_error_log("new client - accept error");
        close(new_sock);
        return ERROR;
    }

    sprintf(temp_log, "new  client - ��client���� %s", inet_ntoa(remote_addr.sin_addr));
    print_log(temp_log);
    print_hint(temp_log);

    FD_SET(new_sock, p_rfd);

    if (new_sock > maxfd)
        maxfd = new_sock;

    // ֻ��ά��fd���ܱ���һ��client������״̬
    OnlineClient *new_client = new OnlineClient(new_sock);
    // ����client_fd��client��ӳ���ϵ
    sock_map_client.insert(make_pair(new_sock, new_client));
    // ���½�����client�ĵ�ַ�ŵ�������
    online_clients.push_back(new_client);
    return OK;
}

Status Server::alive_confirm(OnlineClient * sender_client)
{
    // �ͻ��˷�������������������Ҫ�ظ���ǰ�����б�
    alive_confirm_package pack;
    if (recv(sender_client->sock, &pack, sizeof(alive_confirm_package), 0) <= 0)
    {
        print_error_log("alive confirm - recv error");
        return ERROR;
    }
    sock_map_client[sender_client->sock]->refresh_timer();

    // sprintf(temp_log, "alive confirm - �յ�fd��Ϊ%hd�ſͻ��˵�������", sender_client->sock);
    // print_log(temp_log);

    // ���ͻظ���
    auto it = online_clients.begin();
    int online_num = 0;
    for (; it != online_clients.end(); it++)
    {
        if ((*it)->userid >= 0)
            online_num++;
    }

    available_confirm_package reply_pack(online_num);
    reply_pack.usernum = htons(online_num);

    if (online_num % 2 != 0)
        online_num++;

    int buf_size = online_num * sizeof(short) + sizeof(functional_package) + 2 * sizeof(short);
    char *buf = new char[buf_size];
    int pointer = 0; // buf��ָ��

    memcpy(buf, &reply_pack.func_package, sizeof(functional_package));
    pointer += sizeof(functional_package);

    memcpy(&buf[pointer], &reply_pack.usernum, sizeof(short));
    pointer += sizeof(short);

    memcpy(&buf[pointer], &reply_pack.zero, sizeof(short));
    pointer += sizeof(short);

    it = online_clients.begin();
    for (; it != online_clients.end(); it++)
    {
        if ((*it)->userid >= 0)
        {
            short h_userid = htons((*it)->userid);
            memcpy(&buf[pointer], &h_userid, sizeof(short));
            pointer += sizeof(short);
        }
    }

    if (send(sender_client->sock, buf, buf_size, 0) != buf_size)
    {
        print_error_log("alive confirm - send data to receiver client error");
        return ERROR;
    }

    // sprintf(temp_log, "alive confirm - ��fd��Ϊ%hd�ſͻ��˷��������б�ɹ�", sender_client->sock);
    // print_log(temp_log);

    return OK;
}

Status Server::transfer_message(OnlineClient * sender_client, int pack_length)
{
    sprintf(temp_log, "tranfer msg - �յ�ת����Ϣ����");
    print_log(temp_log);

    msg_package pack;
    int recv_res = recv(sender_client->sock, &pack, pack_length, 0);

    short sender_id = ntohs(pack.sender_id);
    short receiver_id = ntohs(pack.receiver_id);

    if (receiver_id > 0)
    {
        sprintf(temp_log, "tranfer msg - userid = %hd �� userid = %hd ������Ϣ", sender_id, receiver_id);
        print_log(temp_log);

        if (database.add_history(sender_id, receiver_id, time(0), pack.data) == OK)
            print_log("tranfer msg - ��¼��ʷ��Ϣ�ɹ�");
        else
            print_error_log("tranfer msg - ��¼��ʷ��Ϣʧ�ܣ�ȷ���û��Ƿ����");

        if (id_map_client.count(receiver_id) > 0)
        {
            // ����id��Ӧ�Ŀͻ������ߣ����к����Ĳ���
            print_log("tranfer msg - ���շ��ͻ������ߣ�׼��������Ϣ");
            OnlineClient *receiver_client = id_map_client[receiver_id];
            int send_res;
            do
            {
                send_res = send(receiver_client->sock, &pack, pack_length, 0);
                if (send_res <= 0)
                    print_error_log("send data to receiver client error");
            } while (send_res <= 0);

            print_log("tranfer msg - ת����Ϣ�ɹ�");
        }
        else
        {
            // ����id��Ӧ�Ŀͻ��˲����ߣ����к����Ĳ���
            print_log("tranfer msg - ���շ��ͻ������ߣ���ǰ�޷�����");
        }
    }
    else if (receiver_id == 0)
    {
        sprintf(temp_log, "tranfer msg - userid = %hd Ⱥ����Ϣ", sender_id, receiver_id);
        print_log(temp_log);

        if (database.add_history(sender_id, receiver_id, time(0), pack.data) == OK)
            print_log("tranfer msg - ��¼��ʷ��Ϣ�ɹ�");
        else
            print_error_log("tranfer msg - ��¼��ʷ��Ϣʧ�ܣ�ȷ���û��Ƿ����");

        auto it = online_clients.begin();
        for (; it != online_clients.end(); it++)
        {
            receiver_id = (*it)->userid;

            if(receiver_id == sender_client->userid)
                continue;

            if (id_map_client.count(receiver_id) > 0)
            {
                // ����id��Ӧ�Ŀͻ������ߣ����к����Ĳ���
                sprintf(temp_log, "tranfer msg - ���շ��ͻ������ߣ�׼��������Ϣ userid = %hd", receiver_id);
                print_log(temp_log);

                OnlineClient *receiver_client = id_map_client[receiver_id];
                int send_res;
                do
                {
                    send_res = send(receiver_client->sock, &pack, pack_length, 0);
                    if (send_res <= 0)
                        print_error_log("send data to receiver client error");
                } while (send_res <= 0);

                print_log("tranfer msg - ת����Ϣ�ɹ�");
            }
            else
            {
                // ����id��Ӧ�Ŀͻ��˲����ߣ����к����Ĳ���
                print_log("tranfer msg - ���շ��ͻ������ߣ���ǰ�޷�����");
            }
        }
    }
}

Status Server::transfer_filereq(OnlineClient * sender_client, char func_byte, int pack_length)
{
    print_log("filereq - �յ��ļ���������");

    cout<<"length = "<<pack_length<<endl;

    filereq_package pack(func_byte);
    int recv_res = recv(sender_client->sock, &pack, pack_length, 0);

    print_log("filereq - ׼��ת���ļ���������");

    short sender_id = ntohs(pack.sender_id);
    short receiver_id = ntohs(pack.receiver_id);

    int file_size = ntohl(pack.file_size);

    char temp_history[1024];
    if (func_byte == FILE_REQ_TYPE)
        sprintf(temp_history, "request for send file, filename:%s, filesize:%hd", pack.file_name, file_size);
    else if (func_byte == FILE_AGREE_TYPE)
        sprintf(temp_history, "agree to send file, filename:%s, filesize:%hd", pack.file_name, file_size);
    else if (func_byte == FILE_DISAGREE_TYPE)
        sprintf(temp_history, "disagree to send file, filename:%s, filesize:%hd", pack.file_name, file_size);
    else if (func_byte == FILE_STOP_TYPE)
        sprintf(temp_history, "stop send file, filename:%s, filesize:%hd", pack.file_name, file_size);
    else
    {
        print_log("δ֪���ļ���������");
        return ERROR;
    }

    print_log(temp_history);

    if (database.add_history(sender_id, receiver_id, time(0), temp_history) == OK)
        print_log("filereq - ��¼��ʷ��Ϣ�ɹ�");
    else
        print_error_log("filereq - ��¼��ʷ��Ϣʧ�ܣ�ȷ���û��Ƿ����");

    sprintf(temp_log, "filereq - userid = %hd �� userid = %hd �����ļ�����", sender_id, receiver_id);
    print_log(temp_log);

    cout << "file req content: ";
    char *buf = new char[pack_length];
    memcpy(buf, &pack, pack_length);
    for (int i = 0; i < pack_length; i++)
        cout << std::hex << int(buf[i]) << " ";
    cout << endl;

    if (id_map_client.count(receiver_id) > 0)
    {
        // ����id��Ӧ�Ŀͻ������ߣ����к����Ĳ���
        print_log("filereq - ���շ��ͻ������ߣ�׼��ת���ļ������");
        OnlineClient *receiver_client = id_map_client[receiver_id];

        int send_res;
        do
        {
            send_res = send(receiver_client->sock, buf, pack_length, 0);
            if (send_res <= 0)
                print_error_log("filereq - send data to receiver client error");
        } while (send_res <= 0);

        print_log("filereq - ת���ļ�������ɹ�");
    }
    else
    {
        // ����id��Ӧ�Ŀͻ��˲����ߣ����к����Ĳ���
        print_log("filereq - ���շ��ͻ������ߣ���ǰ�޷�����");
    }
}

Status Server::transfer_file(OnlineClient * sender_client, int pack_length)
{
    print_log("file - �յ��ļ�");
    if(pack_length > 1072)
    {
        sprintf(temp_log, "file - �ļ�����С=%d ��������" ,pack_length);
        print_log(temp_log);
        return ERROR;
    }
    cout<<"length = "<<pack_length<<endl;

    file_package pack(pack_length);

    int recv_res = recv(sender_client->sock, &pack, pack_length, 0);

    short sender_id = ntohs(pack.sender_id);
    short receiver_id = ntohs(pack.receiver_id);

    int current_pack = ntohl(pack.current_pack_num)+1;
    int total_pack = ntohl(pack.file_pack_num);
    sprintf(temp_log, "file - user %hd �� user %hd �����ļ�����: %d/%d-%.2f%%", 
            sender_id, 
            receiver_id,
            current_pack,
            total_pack,
            current_pack*1.0/total_pack
            );
    print_log(temp_log);

    if (id_map_client.count(receiver_id) > 0)
    {
        // ����id��Ӧ�Ŀͻ������ߣ����к����Ĳ���
        print_log("file - ���շ��ͻ������ߣ�׼��ת���ļ���");
        OnlineClient *receiver_client = id_map_client[receiver_id];

        int send_res;
        do
        {
            send_res = send(receiver_client->sock, &pack, pack_length, 0);
            if (send_res <= 0)
                print_error_log("send data to receiver client error");
        } while (send_res <= 0);

        print_log("file - ת���ļ����ɹ�");
    }
    else
    {
        // ����id��Ӧ�Ŀͻ��˲����ߣ����к����Ĳ���
        print_log("file - ���շ��ͻ������ߣ���ǰ�޷�����");
    }
}

Status Server::history_req(OnlineClient *sender_client)
{
    // �ͻ�����������ĳ�������û��������¼����������������Ͷ�Ӧ�������¼
    sprintf(temp_log, "history - idΪ%hd�Ŀͻ��˷�����ʷ��¼����", sender_client->userid);
    print_log(temp_log);

    history_ask_package pack;
    if (recv(sender_client->sock, &pack, sizeof(history_ask_package), 0) <= 0)
    {
        sprintf(temp_log, "history - ��idΪ%hd�Ŀͻ��˽��մ���", sender_client->userid);
        print_error_log(temp_log);
        return ERROR;
    }

    short sender_id = (pack.sender_id);
    short receiver_id = (pack.receiver_id);
    short ask_num = (pack.history_ask_num);

    // cout<<"raw sender_id "<<pack.sender_id<<endl;
    // cout<<"new sender_id "<<sender_id<<endl;
    // cout<<"raw receiver_id "<<pack.receiver_id<<endl;
    // cout<<"new receiver_id "<<receiver_id<<endl;
    // cout<<"raw ask_num "<<pack.history_ask_num<<endl;
    // cout<<"new ask_num"<<ask_num<<endl;

    vector<history_reply_package *> reply_packs;
    Status get_his_res = database.get_history(sender_id, receiver_id, ask_num, reply_packs);

    // �����е���ʷ��¼�����͸�����
    int i;
    int total = reply_packs.size();
    sprintf(temp_log, "history - ��ʷ��¼��ȡ��%d��", total);
    print_log(temp_log);

    for (i = 0; i < reply_packs.size(); i++)
    {
        int buf_size = ntohs((reply_packs[i]->func_package).package_length);
        // cout<<"length = "<<buf_size<<endl;
        // cout<<"data: "<<reply_packs[i]->data<<endl;
        // cout<<"time: "<<reply_packs[i]->chat_time<<endl;
        char *buf = new char[buf_size];

        reply_packs[i]->total_num = htons(total);

        memcpy(buf, reply_packs[i], buf_size);

        int non_data_length = sizeof(functional_package) + 4 * sizeof(short) + sizeof(int);
        // cout<<"non_data"<<non_data_length<<endl;
        memcpy(buf, reply_packs[i], non_data_length);
        int data_length = buf_size - non_data_length;
        memcpy(&buf[non_data_length], reply_packs[i]->data, data_length);

        int send_res;
        do
        {
            send_res = send(sender_client->sock, buf, buf_size, 0);
            if (send_res <= 0)
                print_error_log("history - send history to receiver client error");
        } while (send_res <= 0);

        delete buf;
        delete reply_packs[i];
        usleep(10000);
    }
    sprintf(temp_log, "history - ��ʷ��¼���ͳɹ���client id:%hd", receiver_id);
    print_log(temp_log);

    return OK;
}

void Server::server_mainloop()
{
    fd_set rfd, wfd;
    int i, j;

    int maxfd = server_sock;
    FD_ZERO(&rfd);
    // FD_ZERO(&wfd);
    FD_SET(server_sock, &rfd);

    time_t last_time = time(0);
    while (1)
    {
        /* =================================== ������ά�� =================================== */
        FD_ZERO(&rfd);
        // FD_ZERO(&wfd);
        FD_SET(server_sock, &rfd);

        // ����ʱ��
        time_t new_time = time(0);
        time_t delta_time;

        delta_time = new_time - last_time;
        last_time = new_time;

        // ���¼�ʱ�������ĳ���ͻ��˳�ʱ����Ͽ�����
        // ���һ��������Ѿ��������ߵ�client
        for (i = 0; i < online_clients.size(); i++)
        {
            if (online_clients[i]->dead)
            {
                quit_client(online_clients[i]);
                i--;
            }
            else
            {
                bool is_alive = online_clients[i]->update_timer(delta_time);
                if (!is_alive)
                {
                    if (online_clients[i]->userid == -1)
                        sprintf(temp_log, "quit client - sock = %d δ��¼�ҳ�ʱ", online_clients[i]->sock);
                    else
                        sprintf(temp_log, "quit client - userid = %d ���ڳ�ʱ����", online_clients[i]->userid);
                    print_log(temp_log);
                    print_hint(temp_log);
                    quit_client(online_clients[i]);
                    i--;
                }
            }
        }

        maxfd = server_sock;
        for (i = 0; i < online_clients.size(); i++)
        {
            int temp_sock = online_clients[i]->sock;
            // reset fds
            FD_SET(temp_sock, &rfd);
            // FD_SET(temp_sock, &wfd);
            if (temp_sock > maxfd)
                maxfd = temp_sock;
        }

        // // update maxfd
        // int temp_maxfd = 0;
        // for(int i = 0;i <= maxfd;i++)
        // {
        //     if(FD_ISSET(i, &rfd) && i > temp_maxfd)
        //         temp_maxfd = i;
        // }
        // maxfd = temp_maxfd;

        // select all
        int sel = select(maxfd + 1, &rfd, &wfd, NULL, NULL);

        if (sel < 0)
        {
            print_error_log("mainloop - select error");
        }
        else if (sel > 0)
        {
            /* =================================== ������client =================================== */
            if (FD_ISSET(server_sock, &rfd)) // new client
            {
                print_log("new client - ���յ��µ�client��������");

                if (accept_new_client(&rfd, maxfd) == OK)
                {
                    print_log("new client - �µ�client���ӳɹ�");
                }
                else
                {
                    print_log("new client - �µ�client����ʧ��");
                }
            }

            for (i = 0; i < online_clients.size(); i++)
            {
                if (online_clients[i]->sock == server_sock)
                    continue;
                /* =================================== �������ݰ� =================================== */
                if (FD_ISSET(online_clients[i]->sock, &rfd)) // client should read
                {
                    int sender_sock = online_clients[i]->sock;

                    functional_package func_pack;
                    int recv_res = recv(sender_sock, &func_pack, sizeof(functional_package), MSG_PEEK);
                    if (recv_res < 0)
                    {
                        if (errno != EAGAIN)
                        {
                            sprintf(temp_log, "quit client - userid = %d �쳣�˳�recv < 0", online_clients[i]->userid);
                            print_log(temp_log);
                            print_hint(temp_log);

                            online_clients[i]->dead = true;
                            // print_error_log("mainloop - recv peek error");
                        }
                        continue;
                    }
                    else if (recv_res == 0)
                    {
                        sprintf(temp_log, "quit client - userid = %d �쳣�˳�recv = 0", online_clients[i]->userid);
                        print_log(temp_log);
                        print_hint(temp_log);

                        online_clients[i]->dead = true;
                        continue;
                    }

                    char pack_type = func_pack.package_type;
                    char func_byte = func_pack.functional_byte;

                    /* =================================== ɸѡ���ݰ� =================================== */
                    if (pack_type == ALIVE_CONFIRM)
                    {
                        // �ͻ��˷�������������������Ҫ�ظ���ǰ�����б�
                        alive_confirm(online_clients[i]);
                    }
                    else if (pack_type == LOGIN_REQ)
                    {
                        // �ͻ��˷��͵�¼���󣬱�ʾ��ʱ���е�¼
                        login_req(online_clients[i], func_byte);
                    }
                    else if (pack_type == MSG_PACK)
                    {
                        // �ӿͻ����յ��������ݰ���׼������ת��������һ���ͻ���
                        transfer_message(online_clients[i], ntohs(func_pack.package_length));
                    }
                    else if (pack_type == FILE_REQ)
                    {
                        transfer_filereq(online_clients[i], func_byte,ntohs(func_pack.package_length));
                    }
                    else if (pack_type == FILE_PACK)
                    {
                        transfer_file(online_clients[i], ntohs(func_pack.package_length));
                    }
                    else if (pack_type == CONFIG_UPDATE)
                    {
                    }
                    else if (pack_type == QUIT_MESSAGE)
                    {
                        // ���ͻ��˴������б����Ƴ�
                        online_clients[i]->dead = true;
                        sprintf(temp_log, "quit client - userid = %d �˳�", online_clients[i]->userid);
                        print_log(temp_log);
                        print_hint(temp_log);
                    }
                    else if (pack_type == HISTORY_REQ)
                    {
                        history_req(online_clients[i]);
                    }
                    else
                    {
                        char useless_buf;
                        int res = recv(sender_sock, &useless_buf, 1, 0);
                        sprintf(temp_log, "mainloop - ���ݰ�����δ֪��client_fd:%d", sender_sock);
                        print_log(temp_log);
                    }
                }

                /* =================================== ֪ͨ��client =================================== */
                else if (FD_ISSET(online_clients[i]->sock, &wfd))
                {
                }
            }
        }
    }
}
