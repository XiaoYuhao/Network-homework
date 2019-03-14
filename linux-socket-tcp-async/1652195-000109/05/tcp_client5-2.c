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
#define MAX 20

int sock[MAX];
fd_set sock_fd[MAX];

void senddata(){
	char sendbuf[15]="012345678901234";
	int i;
	for(i=0;i<MAX;i++){
		if(sock[i]){
			int ret=send(sock[i],sendbuf,sizeof(sendbuf),MSG_DONTWAIT);
			printf("sock%d send����ֵ��%d\n",i+1,ret);
		}
	}
	alarm(3);//3��֮���ٴλ���
}

int main(int argc,const char * argv[])
{

	if(signal(SIGALRM,senddata)==SIG_ERR)
	{
		perror("signal");
		return 0;
	}
	
	int i;
	int num=argc-2;
	for(i=0;i<num;i++){//ѭ������sock��������Ϊ������״̬
		sock[i]=socket(AF_INET,SOCK_STREAM,0);
		if(sock[i]<0)
		{
			perror("socket");
			return 1;
		}
		int val;
		if(val=fcntl(sock[i],F_GETFL,0)<0){//��ȡ�ļ�״̬��־
			perror("fcntl");
			close(sock[i]);
			return 0;
		}
		if(fcntl(sock[i],F_SETFL,val|O_NONBLOCK)<0){//�����ļ�״̬��־
			perror("fcntl");
			close(sock[i]);
			return 0;
		}
		struct sockaddr_in server;
		server.sin_family=AF_INET;
		server.sin_port=htons(atoi(argv[2+i]));
		server.sin_addr.s_addr=inet_addr(argv[1]);
		socklen_t len=sizeof(struct sockaddr_in);
		
		connect(sock[i],(struct sockaddr*)&server,len);
		printf("sock%d���ӳɹ���\n",i+1);
	}
	alarm(3);
	while(1){
		for(i=0;i<num;i++){
			char recvbuf[100];
			FD_ZERO(&sock_fd[i]);
			FD_SET(sock[i],&sock_fd[i]);
			if(select(sock[i]+1,&sock_fd[i],NULL,NULL,0)>0){
				int ret=recv(sock[i],recvbuf,sizeof(recvbuf),MSG_DONTWAIT);
				printf("sock%d read����ֵ��%d\n",i+1,ret);
			}
		}
	}
	
	for(i=0;i<num;i++){
		close(sock[i]);
	}
	return 0;
}
		