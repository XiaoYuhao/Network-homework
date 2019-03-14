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

int get_local_ip()
{
	struct ifaddrs *ifAddrStruct;
	void *tmpAddrPtr=NULL;
	char ip[INET_ADDRSTRLEN];
	int n=0;
	getifaddrs(&ifAddrStruct);
	while(ifAddrStruct!=NULL)
	{
		if(ifAddrStruct->ifa_addr->sa_family==AF_INET){
			tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			inet_ntop(AF_INET,tmpAddrPtr,ip,INET_ADDRSTRLEN);
			printf("%s IP Address:%s\n",ifAddrStruct->ifa_name,ip);
/*			if(n==0){
				strcat(ips,ip,INET_ADDRSTRLEN);
			}
			else{
				strncat(ips,",",1);
				strncat(ips,ip,INET_ADDRSTRLEN);
			}*/
			n++;
		}
		ifAddrStruct=ifAddrStruct->ifa_next;
	}
	freeifaddrs(ifAddrStruct);
	return 0;
}
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
	get_local_ip();
	
	int listen_sock=startup(atoi(argv[2]),argv[1]);
	
	struct sockaddr_in remote;
	socklen_t len=sizeof(struct sockaddr_in);
	
	int client;
	int iDataNum;
	
	while(1){
		printf("监听端口：%d\n",atoi(argv[2]));
		client=accept(listen_sock,(struct sockaddr*)&remote,&len);
		if(client<0)
		{
			perror("accept");
			continue;
		}
		printf("等待消息...\n");
		printf("get a client,ip:%s,port:%d\n",inet_ntoa(remote.sin_addr),ntohs(remote.sin_port));
		char buf[1024];
		while(1){
			printf("读取消息:");
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
			
			printf("发送消息：");
			scanf("%s",buf);
			printf("\n");
			send(client,buf,strlen(buf),0);
			if(strcmp(buf,"quit")==0)break;
		}
	}
	close(listen_sock);
	return 0;
}