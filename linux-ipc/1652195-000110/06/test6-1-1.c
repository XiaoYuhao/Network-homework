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
#include<sys/un.h>

#define MAXLEN 100

int main(){
	pid_t cpid;
    cpid=fork();
    if(cpid==-1||cpid>0)
        return 0; 
	
	int server_socket,client_socket;
	int server_len,client_len;
	struct sockaddr_un server_address;
	struct sockaddr_un client_address;
	
	unlink("server_socketfd");	
	server_socket=socket(AF_UNIX,SOCK_STREAM,0);
	server_address.sun_family=AF_UNIX;
	strcpy(server_address.sun_path,"server_socketfd");
	server_len=sizeof(server_address);
	
	int val;
	if(val=fcntl(server_socket,F_GETFL,0)<0){//获取文件状态标志
		perror("fcntl");
		close(server_socket);
		return 0;
	}
	if(fcntl(server_socket,F_SETFL,val|O_NONBLOCK)<0){//设置文件状态标志
		perror("fcntl");
		close(server_socket);
		return 0;
	}
	
	if(bind(server_socket,(struct sockaddr *)&server_address,server_len)<0){
		perror("bind");
		exit(1);
	}
	if(listen(server_socket,5)<0){
		perror("listen");
		exit(1);
	}
	
	client_len=sizeof(client_address);
	
	fd_set rfd;
	FD_ZERO(&rfd);
	FD_SET(server_socket,&rfd);
	switch(select(server_socket+1,&rfd,NULL,NULL,NULL))
	{
		case -1:
			perror("select");
			break;
		case 0:
			sleep(1);
			printf("超时！\n");
			break;
		default:
			client_socket=accept(server_socket,(struct sockaddr*)&client_address,&client_len);
	}
	if(client_socket<0){
		perror("accept");
	}
	
	if(val=fcntl(client_socket,F_GETFL,0)<0){//获取文件状态标志
		perror("fcntl");
		close(client_socket);
		return 0;
	}
	if(fcntl(client_socket,F_SETFL,val|O_NONBLOCK)<0){//设置文件状态标志
		perror("fcntl");
		close(client_socket);
		return 0;
	}
	
	fd_set wfd;
	while(1){
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_SET(client_socket,&rfd);
		FD_SET(client_socket,&wfd);
		switch(select(client_socket+1,&rfd,&wfd,NULL,NULL))
		{
			case -1:
				break;
			case 0:
				//sleep(1);
				printf("超时\n");
				break;
			default:
				if(FD_ISSET(client_socket,&rfd)){
					FD_CLR(client_socket,&rfd);
					char recvbuf[MAXLEN];
					recv(client_socket,recvbuf,sizeof(recvbuf),MSG_DONTWAIT);
					printf("%s",recvbuf);
				//	sleep(1);
				}
				if(FD_ISSET(client_socket,&wfd)){
					FD_CLR(client_socket,&wfd);
					char sendbuf[MAXLEN]="test6-1-1 send to test6-1-2\n";
					write(client_socket,sendbuf,sizeof(sendbuf));
					sleep(1);
				}
		}
	}
}