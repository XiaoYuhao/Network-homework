//tcp_server.c
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>

int startup(int _port)
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
	int iDataNum;
	
	while(1){
		printf("�����˿ڣ�%d\n",atoi(argv[1]));
		client=accept(listen_sock,(struct sockaddr*)&remote,&len);
		if(client<0)
		{
			perror("accept");
			continue;
		}
		printf("�ȴ���Ϣ...\n");
		printf("get a client,ip:%s,port:%d\n",inet_ntoa(remote.sin_addr),ntohs(remote.sin_port));
		char buf[1024];
		while(1){
			printf("��ȡ��Ϣ:");
			buf[0]='\0';
			iDataNum=recv(client,buf,1024,0);
			if(iDataNum<0)
			{
				perror("recv null");
				continue;
			}
			buf[iDataNum]='\0';
			if(strcmp(buf,"quit")==0)break;
			printf("%s\n",buf);
			
			printf("������Ϣ��");
			scanf("%s",buf);
			printf("\n");
			send(client,buf,strlen(buf),0);
			if(strcmp(buf,"quit")==0)break;
		}
	}
	close(listen_sock);
	return 0;
}