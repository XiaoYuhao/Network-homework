//tcp_server.c
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<ifaddrs.h>

int startup(int _port,const char *ip)
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
	local.sin_addr.s_addr=inet_addr(ip);
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
	
	if(argc!=3)
	{
		printf("Usage:%s [ip] [loacl_port]\n",argv[0]);
		return 1;
	}
	
	int listen_sock=startup(atoi(argv[2]),argv[1]);
	
	struct sockaddr_in remote;
	socklen_t len=sizeof(struct sockaddr_in);
	
	int client;
	int iDataNum;
	
	printf("监听端口：%d\n",atoi(argv[2]));
	client=accept(listen_sock,(struct sockaddr*)&remote,&len);
	if(client<0)
	{
		perror("accept");
	}
	char buf[1024];
	iDataNum=read(client,buf,20);
	printf("read 返回值：%d\n",iDataNum);
	printf("read 返回内容：%s\n",buf);
		
	return 0;
}