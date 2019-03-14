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
#define MAX 20

int client[MAX]; //client端集合
fd_set client_fds[MAX]; //client端套接字集合
int max_no=0;	//当前client端连接数量

void senddata(){
	char sendbuf[10]="0123456789";
	int i;
	for(i=0;i<MAX;i++){
		if(client[i]){
			int ret=send(client[i],sendbuf,sizeof(sendbuf),MSG_DONTWAIT);
			printf("send返回值：%d\n",ret);
		}
	}
	alarm(1);//1秒之后再次唤醒
}
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
	
	if(signal(SIGALRM,senddata)==SIG_ERR)
	{
		perror("signal");
		return 0;
	}
	
	struct sockaddr_in remote;
	socklen_t len=sizeof(struct sockaddr_in);
	
	
	printf("监听端口：%d\n",atoi(argv[1]));
	alarm(1);//设置闹钟
	
	fd_set rfd;
	int i,max_fd=0;
	for(i=0;i<MAX;i++){//初始化client集合
		client[i]=0;
	}
	while(1){
		FD_ZERO(&rfd);
		FD_SET(listen_sock,&rfd);
		if(listen_sock>max_fd) max_fd=listen_sock;
		
		int conn=-1;
		if(select(max_fd+1,&rfd,NULL,NULL,0)>0){//可能出现一个连接请求
			conn=accept(listen_sock,(struct sockaddr*)&remote,&len);//接受连接
		}
		if(conn>0){//成功连接
			for(i=0;i<MAX;i++){
				if(!client[i]){
					client[i]=conn;//将连接成功的conn加入client集合中
					if(client[i]>max_fd) max_fd=client[i];//更新max_fd
					int val;
					if(val=fcntl(client[i],F_GETFL,0)<0){//获取文件状态标志
						perror("fcntl");
						close(client[i]);
						return 0;
					}
					if(fcntl(client[i],F_SETFL,val|O_NONBLOCK)<0){//设置文件状态标志
						perror("fcntl");
						close(client[i]);
						return 0;
					}
					max_no=i+1;
					printf("第%d个client已经连接\n",max_no);
					break;
				}
			}
		}
		for(i=0;i<MAX;i++){
			if(client[i]){
				char recvbuf[100];
				FD_ZERO(&client_fds[i]);
				FD_SET(client[i],&client_fds[i]);
				if(select(client[i]+1,&client_fds[i],NULL,NULL,0)>0){
					int ret=recv(client[i],recvbuf,sizeof(recvbuf),MSG_DONTWAIT);
					printf("read返回值：%d\n",ret);
				}
			}
			if(i==max_no)printf("当前client连接数为%d\n",max_no);
		}
	}
	
	close(listen_sock);
	return 0;
}