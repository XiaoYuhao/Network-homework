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
	
	int nRecvBuf=64*1024;
	setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
	int nSendBuf=64*1024;
	setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
	
	if(connect(sock,(struct sockaddr*)&server,len)<0)
	{
			perror("connect");
			return 2;
	}
	
	char sendbuf[1024];
	char recvbuf[1024];
	int iDataNum;
	int nbyte=0;
	char *buf="abcdefghijklmnopqrstuvwxyz";
	while(1){
		int ret;
		ret=write(sock,buf,strlen(buf));
		if(ret<0){
			printf("write失败！\n");
			exit(1);
		}
		else if(ret==0){
			printf("write缓冲区已满！\n");
			fflush(stdout);
		}
		else{
		nbyte+=ret;
		printf("已write：%d字节\n",nbyte);
		}
	}
	
	
	close(sock);
	return 0;
}
	