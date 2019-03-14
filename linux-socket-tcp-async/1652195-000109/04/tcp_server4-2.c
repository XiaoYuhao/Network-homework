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
#include<time.h>
#include<signal.h>

int startup(int _port)
{
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		perror("socket");
		exit(1);
	}
	int val;
	if(val=fcntl(sock,F_GETFL,0)<0){//获取文件状态标志
		perror("fcntl");
		close(sock);
		return 0;
	}
	if(fcntl(sock,F_SETFL,val|O_NONBLOCK)<0){//设置文件状态标志
		perror("fcntl");
		close(sock);
		return 0;
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
	fd_set rfd;
	FD_ZERO(&rfd);
	FD_SET(listen_sock,&rfd);
	switch(select(listen_sock+1,&rfd,NULL,NULL,NULL))
	{
		case -1:
			perror("select");
			break;
		case 0:
			sleep(1);
			printf("超时！\n");
			break;
		default:
			client=accept(listen_sock,(struct sockaddr*)&remote,&len);
	}
	
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
	
	
	char recvbuf[20];
	int rDataNum=0;
	
	int ret;
	while(1){
		FD_ZERO(&rfd);
		FD_SET(client,&rfd);
		switch(select(client+1,&rfd,NULL,NULL,NULL))
		{
			case -1:
				break;
			case 0:
				sleep(1);
				printf("超时\n");
				break;
			default:
				ret=recv(client,recvbuf,sizeof(recvbuf),MSG_DONTWAIT);
				rDataNum+=ret;
				printf("已read：%d\n",rDataNum);
		}
		sleep(1);
	}
	
	close(listen_sock);
	return 0;
}