//tcp_server.c
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<fcntl.h>
#include<sys/shm.h>
int startup(int _port)
{
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		perror("socket");
		exit(1);
	}
	
	struct sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_port=htons(_port);
	local.sin_addr.s_addr=htonl(INADDR_ANY);
	socklen_t len=sizeof(local);
	
	if(bind(sock,(struct sockaddr*)&local,len)<0)
	{
		perror("bind");
		exit(2);
	}
	
	if(listen(sock,5)<0)
	{
		perror("listen");
		exit(3);
	}
	
	return sock;
}

int main(int argc,const char * argv[])
{
	
	if(argc!=2)
	{
		printf("Usage:%s [loacl_port]\n",argv[0]);
		return 1;
	}
	
	int listen_sock=startup(atoi(argv[1]));
	
	struct sockaddr_in remote;
	socklen_t len=sizeof(struct sockaddr_in);
	
	int client;
	
	printf("监听端口：%d\n",atoi(argv[1]));
	client=accept(listen_sock,(struct sockaddr*)&remote,&len);
	if(client<0)
	{
		perror("accept");
	}
	int val;
	if(val=fcntl(client,F_GETFL,0)<0){//获取文件状态标志
		perror("fcntl");
		close(client);
		return 0;
	}
	if(fcntl(client,F_SETFL,val|O_NONBLOCK)<0){//设置文件状态标志
		perror("fcntl");
		close(client);
		return 0;
	}
	
	
	char sendbuf[1024];
	char recvbuf[1024];
	int iDataNum;
	
	int ret=read(client,recvbuf,sizeof(recvbuf));
	printf("read返回值：%d\n",ret);
	
	close(listen_sock);
	return 0;
}