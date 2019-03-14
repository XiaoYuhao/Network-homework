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
#include<time.h>
#include<signal.h>
int sock;

void senddata(){
	char sendbuf[15]="012345678901234";
	int ret=send(sock,sendbuf,sizeof(sendbuf),MSG_DONTWAIT);
	printf("send����ֵ��%d\n",ret);
	alarm(3);//3��֮���ٴλ���
}

int main(int argc,const char * argv[])
{
	if(argc!=3)
	{
		printf("Usage:%s [ip] [port]\n",argv[0]);
		return 0;
	}
	
	sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		perror("socket");
		return 1;
	}
	
	int val;
	if(val=fcntl(sock,F_GETFL,0)<0){//��ȡ�ļ�״̬��־
		perror("fcntl");
		close(sock);
		return 0;
	}
	if(fcntl(sock,F_SETFL,val|O_NONBLOCK)<0){//�����ļ�״̬��־
		perror("fcntl");
		close(sock);
		return 0;
	}
	
	if(signal(SIGALRM,senddata)==SIG_ERR)
	{
		perror("signal");
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
			printf("��ʱ��\n");
			break;
		default:
			connect(sock,(struct sockaddr*)&server,len);
	}
	
	printf("���ӳɹ���\n");
	
	alarm(3);
	
	char recvbuf[100];
	int iDataNum;
	
	int ret;
	
	while(1){
		FD_ZERO(&rfd);
		FD_SET(sock,&rfd);
		switch(select(sock+1,&rfd,NULL,NULL,NULL))
		{
			case -1:
				break;
			case 0:
				sleep(1);
				printf("��ʱ\n");
				break;
			default:
				ret=recv(sock,recvbuf,sizeof(recvbuf),MSG_DONTWAIT);
				printf("read����ֵ��%d\n",ret);
		}
	}

	close(sock);
	return 0;
}
		