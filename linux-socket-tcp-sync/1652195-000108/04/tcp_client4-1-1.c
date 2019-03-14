//client.c
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>

int main(int argc,const char * argv[])
{
	if(argc!=3)
	{
		printf("Usage:%s [ip] [port]\n",argv[0]);
		return 0;
	}
	
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		perror("socket");
		return 1;
	}
	
	struct sockaddr_in server;
	server.sin_family=AF_INET;
	server.sin_port=htons(atoi(argv[2]));
	server.sin_addr.s_addr=inet_addr(argv[1]);
	socklen_t len=sizeof(struct sockaddr_in);
	
	if(connect(sock,(struct sockaddr*)&server,len)<0)
	{
			perror("connect");
			return 2;
	}
	
	char sendbuf[1024];
	char recvbuf[1024];
	int iDataNum;
	char *buf="abcdefghijklmnopqrstuvwxyz";
	write(sock,buf,sizeof(buf));
	
	close(sock);
	return 0;
}
	