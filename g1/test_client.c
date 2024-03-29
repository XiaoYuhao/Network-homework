//client.c
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#define MAXLEN 128

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
	
	char sendbuf[MAXLEN];
	char recvbuf[MAXLEN];
	int iDataNum;
	while(1)
	{
		
		//printf("发送消息：\n");
		scanf("%s",sendbuf);
		printf("\n");
		send(sock,sendbuf,strlen(sendbuf),0);
		if(strcmp(sendbuf,"end")==0)break;
		printf("读取消息：");
		//recvbuf[0]='\0';
		iDataNum=recv(sock,recvbuf,200,0);
		recvbuf[iDataNum]='\0';
		printf("%s\n",recvbuf);
		
	}
	close(sock);
	return 0;
}
	