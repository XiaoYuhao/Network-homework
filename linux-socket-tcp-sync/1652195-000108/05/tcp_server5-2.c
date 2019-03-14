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
	
	printf("¼àÌý¶Ë¿Ú£º%d\n",atoi(argv[2]));
	client=accept(listen_sock,(struct sockaddr*)&remote,&len);
	if(client<0)
	{
		perror("accept");
	}
	
	char sendbuf[1024];
	char recvbuf[1024];
	int nbyte=0;
	char *buf="abcdefghijklmnopqrstuvwxyz";
	while(1){
		int ret;
		ret=write(client,buf,strlen(buf));
		if(ret<0){
			printf("writeÊ§°Ü£¡\n");
			exit(1);
		}
		else if(ret==0){
			printf("write»º³åÇøÒÑÂú£¡\n");
			fflush(stdout);
		}
		else{
		nbyte+=ret;
		printf("ÒÑwrite£º%d×Ö½Ú\n",nbyte);
		}
	}
		
	return 0;
}