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
	if(argc!=5)
	{
		printf("Usage:%s [ip] [port] [rNum] [wNum]\n",argv[0]);
		return 0;
	}
	int rNum=atoi(argv[3]);
	int wNum=atoi(argv[4]);
	
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
	
	char buf[1024*64];
	char *wbuf="abcdefghijklmnopqrstuvwxyz";
	int nbyte=0;
	int wbyte=0;
	while(1){
		int rret,wret;
		wret=write(sock,wbuf,wNum);
		if(wret<0){
			printf("write失败！\n");
			exit(1);
		}
		wbyte+=wret;
		printf("已写入：%d字节\n",wbyte);
		sleep(2);
		rret=read(sock,buf,rNum);
		if(rret<0){
			printf("read出错！\n");
			exit(1);
		}
		nbyte+=rret;
		printf("已读取：%d字节\n",nbyte);
			
	}
	
	
	close(sock);
	return 0;
}
	