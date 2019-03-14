//client.c
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<fcntl.h>
#include<sys/shm.h>
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
	
	struct sockaddr_in server;
	server.sin_family=AF_INET;
	server.sin_port=htons(atoi(argv[2]));
	server.sin_addr.s_addr=inet_addr(argv[1]);
	socklen_t len=sizeof(struct sockaddr_in);
	
	fd_set rfd;
	FD_ZERO(&rfd);
	FD_SET(sock,&rfd);
	switch(select(sock+1,&rfd,NULL,NULL,NULL))
	{
		case -1:
			perror("select");
			break;
		case 0:
			printf("超时！\n");
			break;
		default:
			connect(sock,(struct sockaddr*)&server,len);
	}
	
	printf("连接成功！\n");
	
	char sendbuf[10]="0123456789";
	int iDataNum;
	
	int ret;
	
	fd_set wfd;
	while(1){
		FD_ZERO(&wfd);
		FD_SET(sock,&wfd);
		switch(select(sock+1,NULL,&wfd,NULL,NULL))
		{
			case -1:
				perror("select");
				break;
			case 0:
				sleep(1);
				printf("超时\n");
				break;
			default:
				ret=send(sock,sendbuf,sizeof(sendbuf),0);
				printf("send返回值：%d\n",ret);
		}
		sleep(1);
	}

	close(sock);
	return 0;
}
		