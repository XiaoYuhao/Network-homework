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
	
	if(argc!=4)
	{
		printf("Usage:%s [loacl_port] [rNum] [wNum]\n",argv[0]);
		return 1;
	}
	int rNum=atoi(argv[2]);
	int wNum=atoi(argv[3]);
	
	int listen_sock=startup(atoi(argv[1]),"192.168.93.3");
	
	struct sockaddr_in remote;
	socklen_t len=sizeof(struct sockaddr_in);
	
	int client;
	int iDataNum;
	
	printf("监听端口：%d\n",atoi(argv[1]));
	client=accept(listen_sock,(struct sockaddr*)&remote,&len);
	if(client<0)
	{
		perror("accept");
	}

	int nRecvBuf=64*1024;
	setsockopt(listen_sock,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
	int nSendBuf=64*1024;
	setsockopt(listen_sock,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
	
	char buf[1024*64];
	char *wbuf="abcdefghijklmnopqrstuvwxyz";
	int nbyte=0;
	int wbyte=0;
	while(1){
		int rret,wret;
		rret=read(client,buf,rNum);
		if(rret<0){
			printf("read出错！\n");
			exit(1);
		}
		nbyte+=rret;
		printf("已读取：%d字节\n",nbyte);
		wret=write(client,wbuf,wNum);
		if(wret<0){
			printf("write失败！\n");
			exit(1);
		}
		wbyte+=wret;
		printf("已写入：%d字节\n",wbyte);
		
	}
		
	return 0;
}