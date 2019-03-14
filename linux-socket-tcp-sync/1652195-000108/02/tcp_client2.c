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
	if(argc!=4)
	{
		printf("Usage:%s [client_port] [ip] [server_port]\n",argv[0]);
		return 0;
	}
	
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		perror("socket");
		return 1;
	}
	struct sockaddr_in clientAddr;
	clientAddr.sin_family=AF_INET;
	clientAddr.sin_port=htons(atoi(argv[1]));
	clientAddr.sin_addr.s_addr=0;
	if(bind(sock,(struct sockaddr*)&clientAddr,sizeof(clientAddr))<0){
		perror("bind");
		exit(2);
	}

	
	struct sockaddr_in server;
	server.sin_family=AF_INET;
	server.sin_port=htons(atoi(argv[3]));
	server.sin_addr.s_addr=inet_addr(argv[2]);
	socklen_t len=sizeof(struct sockaddr_in);
	
	if(connect(sock,(struct sockaddr*)&server,len)<0)
	{
			perror("connect");
			return 2;
	}
	
	char sendbuf[1024];
	char recvbuf[1024];
	int iDataNum;
	while(1)
	{
		
		printf("发送消息：\n");
		scanf("%s",sendbuf);
		printf("\n");
		send(sock,sendbuf,strlen(sendbuf),0);
		if(strcmp(sendbuf,"quit")==0)break;
		printf("读取消息：");
		recvbuf[0]='\0';
		iDataNum=recv(sock,recvbuf,200,0);
		recvbuf[iDataNum]='\0';
		printf("%s\n",recvbuf);
		
	}
	close(sock);
	return 0;
}
	
