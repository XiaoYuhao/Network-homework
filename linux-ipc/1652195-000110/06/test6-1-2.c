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
	
	int sock;
	int len;
	struct sockaddr_un address;
	
	sock=socket(AF_UNIX,SOCK_STREAM,0);
	
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
	
	address.sun_family=AF_UNIX;
	strcpy(address.sun_path,"server_socketfd");
	len=sizeof(address);
	
	fd_set rfd;
	FD_ZERO(&rfd);
	FD_SET(sock,&rfd);
	int ret;
	switch(select(sock+1,&rfd,NULL,NULL,NULL))
	{
		case -1:
			perror("select");
			break;
		case 0:
			printf("超时\n");
			break;
		default:
			ret=connect(sock,(struct sockaddr*)&address,len);
	}
	
	if(ret==-1){
		perror("client connect");
		exit(1);
	}
	
	fd_set wfd;
	int sum=0;
	while(1){
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_SET(sock,&rfd);
		FD_SET(sock,&wfd);
		switch(select(sock+1,&rfd,&wfd,NULL,NULL))
		{
			case -1:
				break;
			case 0:
				sleep(1);
				printf("超时\n");
				break;
			default:
				if(FD_ISSET(sock,&rfd)){
					FD_CLR(sock,&rfd);
					char recvbuf[MAXLEN];
					recv(sock,recvbuf,sizeof(recvbuf),MSG_DONTWAIT);
					printf("%s",recvbuf);
				}
				if(FD_ISSET(sock,&wfd)){
					FD_CLR(sock,&wfd);
					char sendbuf[MAXLEN]="test6-1-2 send to test6-1-1\n";
					int ret=send(sock,sendbuf,sizeof(sendbuf),MSG_DONTWAIT);
					sum+=ret;
					printf("已写入%d字节\n",sum);
					
				}
		}
	}
	return 0;
}