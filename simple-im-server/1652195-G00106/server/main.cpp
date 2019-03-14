
#include "Database.hpp"
#include "Server.hpp"
#include "OnlineClient.hpp"
#include "package.hpp"


using namespace std;

int main()
{
    daemon(1,1);    
    Server *chat_server;


    chat_server = new Server(21523);
    chat_server->init_socket();
    chat_server->server_mainloop();
    return 0;
}