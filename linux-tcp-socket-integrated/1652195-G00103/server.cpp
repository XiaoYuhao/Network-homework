


//tcp_server.cpp
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
#define MAX 4096
#define MAXLEN 128
#define maxlen 100008


struct Client_data{
	//char stuno[MAXLEN];
	int stuno;
	//char pid[MAXLEN];
	int pid;
	char time[MAXLEN];
//	char num[maxlen];
	char *str;
	int len;
};

int startup_block(int _port,const char *ip){
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		perror("socket");
		exit(1);
	}
	int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)); 
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

int startup_noblock(int _port,const char *ip){
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		perror("socket");
		exit(1);
	}
	int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)); 
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

int fork_block_work(int port,const char *ip){
		
	int listen_sock=startup_block(port,ip);

	struct sockaddr_in remote;
	socklen_t len=sizeof(struct sockaddr_in);
	
	int sum=1;
	
	printf("监听端口：%d\n",port);
	while(1){
		int conn=-1;
		conn=accept(listen_sock,(struct sockaddr*)&remote,&len);//接受连接
		if(conn<0){
			perror("accept");
			return -1;
		}
		if(conn>0){
			printf("client[%d]连接已经建立\n",conn);
			pid_t pid;
			pid=fork();
			if(pid<0){
				perror("fork");
				return -1;
			}
			if(pid==0){//子进程
				int work=0,ret;
				Client_data client_data;//存放从client端收到的数据
				int client_fd;
				while(1){
					if(work==1){//接收client发送的学号
						int a;
						ret=read(conn,(void*)&a,sizeof(a));
						if(ret<0){
							perror("read");
							close(conn);
							return 0;
						}
						if(ret==0){
							printf("client[%d]异常中断\n",conn);
							close(conn);
							return 0;
						}
						client_data.stuno=a;
						work=2;
					}
					else if(work==3){//接收client发送的pid
						int a;
						ret=read(conn,(void*)&a,sizeof(a));
						if(ret<0){
							perror("read");
							close(conn);
							return 0;
						}
						if(ret==0){
							printf("client[%d]异常中断\n",conn);
							close(conn);
							return 0;
						}
						client_data.pid=a;
						work=4;
					}
					else if(work==5){//接收client发送的系统时间
						ret=read(conn,client_data.time,sizeof(client_data.time));
						if(ret<0){
							perror("read");
							close(conn);
							return 0;
						}
						if(ret==0){
							printf("client[%d]异常中断\n",conn);
							close(conn);
							return 0;
						}
						client_data.time[ret]='\0';
						work=6;
					}
					else if(work==7){//接收client发送的随机字符串
						client_data.str=new char[maxlen];
						int sum=0;
						while(sum<client_data.len){
							ret=read(conn,client_data.str+sum,maxlen);
							if(ret<=0){
								printf("client[%d]异常中断\n",conn);
								delete client_data.str;
								close(conn);
								return 0;
							}
							sum+=ret;
						}
					//	printf("接受字符串长度:%d\n",sum);
						work=8;
					}
					else if(work==9){
						char recvbuf[MAXLEN];
						ret=read(conn,recvbuf,sizeof(recvbuf));
						if(ret==0){//read收到0表示client端已经断开		
							printf("client%d已正常断开连接\n",conn);						
							char fname[MAXLEN];
							sprintf(fname,"txt/%d.%d.pid.txt",ntohl(client_data.stuno),ntohl(client_data.pid));
							client_fd=open(fname,O_CREAT|O_RDWR,0777);
							if(client_fd<0){
								perror("open");
								return 0;
							}
							write(client_fd,&client_data.stuno,sizeof(client_data.stuno));
							write(client_fd,"\n",1);
							write(client_fd,&client_data.pid,sizeof(client_data.pid));
							write(client_fd,"\n",1);
							write(client_fd,client_data.time,strlen(client_data.time));
							write(client_fd,"\n",1);
							write(client_fd,client_data.str,client_data.len);
							write(client_fd,"\n",1);
							//printf("%dclient的str长度:%d\n",conn,client_data.len);
							close(client_fd);
							delete client_data.str;
							close(conn);
							exit(1);//退出子进程
						}
						else{
									//出错处理
						}
					}
					if(work==0){
						char sendbuf[MAXLEN]="StuNo";//发送StuNo
						write(conn,sendbuf,strlen(sendbuf));
						work=1;
					}
					else if(work==2){
						char sendbuf[MAXLEN]="pid";//发送pid
						write(conn,sendbuf,strlen(sendbuf));
						work=3;
					}
					else if(work==4){
						char sendbuf[MAXLEN]="TIME";//发送TIME
						write(conn,sendbuf,strlen(sendbuf)+1);
						work=5;
					}
					else if(work==6){
						srand(getpid());
						int num=rand()%100000;
						if(num<32768)num+=32768;
						char sendbuf[MAXLEN];
						client_data.len=num;
						sprintf(sendbuf,"str%d",num);//发送随机数
						//printf("sendbuf:%s\n",sendbuf);
						write(conn,sendbuf,strlen(sendbuf)+1);
						work=7;
					}
					else if(work==8){
						char sendbuf[MAXLEN]="end";//发送end
						write(conn,sendbuf,strlen(sendbuf));
						work=9;
					}
				}
			
			}
			else{
				close(conn);//父进程关闭conn连接
			}
		}
	}
	close(listen_sock);
	return 1;
}
int fork_work(int port,const char *ip) {
	
	int listen_sock=startup_noblock(port,ip);

	struct sockaddr_in remote;
	socklen_t len=sizeof(struct sockaddr_in);
	
	printf("监听端口：%d\n",port);
	
	fd_set rfd;
	while(1){
		FD_ZERO(&rfd);
		FD_SET(listen_sock,&rfd);
		
		int conn=-1;
		if(select(listen_sock+1,&rfd,NULL,NULL,NULL)>0){//可能出现一个连接请求
			conn=accept(listen_sock,(struct sockaddr*)&remote,&len);//接受连接
		}
		
		if(conn>0){
			pid_t pid;
			pid=fork();
			if(pid<0){
				perror("fork");
				return -1;
			}
			if(pid==0){//子进程
				int val;
				if(val=fcntl(conn,F_GETFL,0)<0){//获取文件状态标志
					perror("fcntl");
					close(conn);
					return -1;
				}
				if(fcntl(conn,F_SETFL,val|O_NONBLOCK)<0){//设置文件状态标志
					perror("fcntl");
					close(conn);
					return -1;
				}
				fd_set rfds,wfds;
				int work=0,ret;
				Client_data client_data;//存放从client端收到的数据
				int client_fd;	
				while(1){
					FD_ZERO(&rfds);
					FD_ZERO(&wfds);
					FD_SET(conn,&rfds);
					FD_SET(conn,&wfds);
					if(select(conn+1,&rfds,&wfds,NULL,NULL)>0){
						if(FD_ISSET(conn,&rfds)){
							FD_CLR(conn,&rfds);
							if(work==1){//接收client发送的学号
								int a;
								ret=read(conn,(void*)&a,sizeof(a));
								if(ret<0){
									perror("read");
									return 0;
								}
								if(ret==0){
									printf("client[%d]异常中断\n",conn);								
									return 0;
								}
								client_data.stuno=a;
								work=2;
							}
							else if(work==3){//接收client发送的pid
								int a;
								ret=read(conn,(void*)&a,sizeof(a));
								if(ret<0){
									perror("read");
									return 0;
								}
								if(ret==0){
									printf("client[%d]异常中断\n",conn);
									return 0;
								}
								client_data.pid=a;
								work=4;
							}
							else if(work==5){//接收client发送的系统时间
								ret=read(conn,client_data.time,sizeof(client_data.time));
								if(ret<0){
									perror("read");
									return 0;
								}
								if(ret==0){
									printf("client[%d]异常中断\n",conn);
									return 0;
								}
								client_data.time[ret]='\0';
								work=6;
							}
							else if(work==7){//接收client发送的随机字符串
								client_data.str=new char[maxlen];
								int sum=0;
								while(sum<client_data.len){
									ret=read(conn,client_data.str+sum,maxlen);
									if(ret<=0){
										printf("client[%d]异常中断\n",conn);
										delete client_data.str;
										return 0;
									}
									sum+=ret;
								}
								work=8;
							}
							else if(work==9){
								char recvbuf[MAXLEN];
								ret=read(conn,recvbuf,sizeof(recvbuf));
								if(ret==0){//read收到0表示client端已经断开	
									printf("client%d已正常断开连接\n",conn);
									char newfname[MAXLEN];
									sprintf(newfname,"txt/%d.%d.pid.txt",ntohl(client_data.stuno),ntohl(client_data.pid));
									client_fd=open(newfname,O_CREAT|O_RDWR,0777);
									if(client_fd<0){
										perror("open");
										return 0;
									}
									write(client_fd,&client_data.stuno,sizeof(client_data.stuno));
									write(client_fd,"\n",1);
									write(client_fd,&client_data.pid,sizeof(client_data.pid));
									write(client_fd,"\n",1);
									write(client_fd,client_data.time,strlen(client_data.time));
									write(client_fd,"\n",1);
									write(client_fd,client_data.str,client_data.len);
									write(client_fd,"\n",1);
									close(client_fd);
									delete client_data.str;
									close(conn);
									exit(1);//退出子进程
								}
								else{
									exit(1);//出错处理
								}
							}
						}
						if(FD_ISSET(conn,&wfds)){
							FD_CLR(conn,&wfds);
							if(work==0){
								char sendbuf[MAXLEN]="StuNo";//发送StuNo
								write(conn,sendbuf,strlen(sendbuf));
								work=1;
							}
							else if(work==2){
								char sendbuf[MAXLEN]="pid";//发送pid
								write(conn,sendbuf,strlen(sendbuf));
								work=3;
							}
							else if(work==4){
								char sendbuf[MAXLEN]="TIME";//发送TIME
								write(conn,sendbuf,strlen(sendbuf)+1);
								work=5;
							}
							else if(work==6){
								srand(getpid());
								int num=rand()%100000;
								if(num<32768)num+=32768;
								client_data.len=num;
								char sendbuf[MAXLEN];
								sprintf(sendbuf,"str%d",num);//发送随机数
								write(conn,sendbuf,strlen(sendbuf)+1);
								work=7;
							}
							else if(work==8){
								char sendbuf[MAXLEN]="end";//发送end
								write(conn,sendbuf,strlen(sendbuf));
								work=9;
							}
						}
					}
				}
			}
			else {//父进程
				close(conn);//父进程关闭conn连接
			}
		}
	}
	close(listen_sock);
	return 1;
}

int nofork_work(int port,const char *ip){
	int client[MAX]; //client端集合
	int client_work[MAX];
	int client_fd[MAX];
	Client_data client_data[MAX];//记录从client端收到的数据
	int ret;
	fd_set client_rfds[MAX]; //client端套接字集合
	fd_set client_wfds[MAX];
	int max_no=0;	//当前client端连接数量
	
	int listen_sock=startup_noblock(port,ip);
	
	struct sockaddr_in remote;
	socklen_t len=sizeof(struct sockaddr_in);
	
	printf("监听端口：%d\n",port);
	
	fd_set rfd;
	fd_set wfd;
//	fd_set client_rfds,client_wfds;
	int max_fd=0;
	for(int i=0;i<MAX;i++){//初始化client集合
		client[i]=0;
		client_work[i]=0;
		client_fd[i]=0;
	}
	
	while(1){
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_SET(listen_sock,&rfd);
		if(listen_sock>max_fd) max_fd=listen_sock;
		for(int i=0;i<MAX;i++){
			if(client[i]){
				FD_SET(client[i],&rfd);
				FD_SET(client[i],&wfd);
				if(client[i]>max_fd)max_fd=client[i];
			}
		}
		int conn=-1;
		//printf("1\n");
		if(select(max_fd+1,&rfd,&wfd,NULL,NULL)>0){//可能出现一个连接请求
			if(FD_ISSET(listen_sock,&rfd)){
				FD_CLR(listen_sock,&rfd);
				conn=accept(listen_sock,(struct sockaddr*)&remote,&len);//接受连接,成功返回非负值，失败返回-1
				if(conn<0){
					perror("accept");
				}
				if(conn>0){//成功连接
					for(int i=0;i<MAX;i++){
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
			}
			
			for(int i=0;i<MAX;i++){
				if(client[i]){
					if(FD_ISSET(client[i],&rfd)){//client[i]有可读的数据
						FD_CLR(client[i],&rfd);
						if(client_work[i]==1){//接收client发送的学号
							int a;
							ret=read(client[i],(void*)&a,sizeof(a));
							if(ret<=0){
								printf("client[%d]异常中断\n",client[i]);
								close(client[i]);
								client[i]=0;//释放
								client_fd[i]=0;
								client_work[i]=0;
								continue;
							}
							client_data[i].stuno=a;
							client_work[i]=2;
						}
						else if(client_work[i]==3){//接收client发送的pid
							int a;
							ret=read(client[i],(void*)&a,sizeof(a));
							if(ret<=0){
								printf("client[%d]异常中断\n",client[i]);
								close(client[i]);
								client[i]=0;//释放
								client_fd[i]=0;
								client_work[i]=0;
								continue;
							}
							client_data[i].pid=a;
							client_work[i]=4;
						}
						else if(client_work[i]==5){//接收client发送的系统时间
							ret=read(client[i],client_data[i].time,sizeof(client_data[i].time));
							if(ret<=0){
								printf("client[%d]异常中断\n",client[i]);
								close(client[i]);
								client[i]=0;//释放
								client_fd[i]=0;
								client_work[i]=0;
								continue;
							}
							client_data[i].time[ret]='\0';
							client_work[i]=6;
						}
						else if(client_work[i]==7){//接收client发送的随机字符串
							client_data[i].str=new char[maxlen];
							int sum=0;
							int flag=0;
							while(sum<client_data[i].len){
								ret=read(client[i],client_data[i].str+sum,maxlen);
								if(ret<=0){
									printf("client[%d]异常中断\n",client[i]);
								//	printf("??????\n");
									delete client_data[i].str;
									close(client[i]);
									client[i]=0;//释放
									client_fd[i]=0;
									client_work[i]=0;
									flag=1;
									break;
									}
									sum+=ret;
							}
							if(flag==1)continue;
							printf("Read:%d\n",sum);
							client_data[i].len=ret;
							//client_data[i].str[ret]='\0';
							client_work[i]=8;
						}
						else if(client_work[i]==9){
							char recvbuf[MAXLEN];
							ret=read(client[i],recvbuf,sizeof(recvbuf));
							if(ret==0){//read收到0表示client端已经断开			
								char newfname[MAXLEN];
								sprintf(newfname,"txt/%d.%d.pid.txt",ntohl(client_data[i].stuno),ntohl(client_data[i].pid));
								client_fd[i]=open(newfname,O_CREAT|O_RDWR,0777);
								if(client_fd[i]<0){
									perror("open");
								}
								write(client_fd[i],&client_data[i].stuno,sizeof(client_data[i].stuno));
								write(client_fd[i],"\n",1);
								write(client_fd[i],&client_data[i].pid,sizeof(client_data[i].pid));
								write(client_fd[i],"\n",1);
								write(client_fd[i],client_data[i].time,strlen(client_data[i].time));
								write(client_fd[i],"\n",1);
								write(client_fd[i],client_data[i].str,client_data[i].len);
								write(client_fd[i],"\n",1);
								if(close(client_fd[i])<0){
									perror("close fd");
								}
								if(close(client[i])){
									perror("close client");
								}
								printf("client[%d]正常退出\n",client[i]);
								delete client_data[i].str;
								client[i]=0;
								client_fd[i]=0;
								client_work[i]=0;
								continue;
							}
							else{
								//出错处理
							}
						}
						
					}
					if(FD_ISSET(client[i],&wfd)){//client[i]可写
						FD_CLR(client[i],&wfd);
						if(client_work[i]==0){
							char sendbuf[MAXLEN]="StuNo";//发送StuNo
							write(client[i],sendbuf,strlen(sendbuf));
							client_work[i]=1;
						}
						else if(client_work[i]==2){
							char sendbuf[MAXLEN]="pid";//发送pid
							write(client[i],sendbuf,strlen(sendbuf));
							client_work[i]=3;
						}
						else if(client_work[i]==4){
							char sendbuf[MAXLEN]="TIME";//发送TIME
							write(client[i],sendbuf,strlen(sendbuf)+1);
							client_work[i]=5;
						}
						else if(client_work[i]==6){
							int num=rand()%100000;
							if(num<32768)num+=32768;
							client_data[i].len=num;
							char sendbuf[MAXLEN];
							sprintf(sendbuf,"str%d",num);//发送随机数
							write(client[i],sendbuf,strlen(sendbuf)+1);
							client_work[i]=7;
						}
						else if(client_work[i]==8){
							char sendbuf[MAXLEN]="end";//发送end
							write(client[i],sendbuf,strlen(sendbuf));
							client_work[i]=9;
						}
					}	
				}
				//if(i==max_no)printf("当前client连接数为%d\n",max_no);
			}
		}
	}
	close(listen_sock);
	return 1;
}

int get_parameter(int argc, const char **argv, char *ip, int *port, int *isblock, int *isfork)
{
	int i;
	int flags[4];
	memset(flags, 0, sizeof(flags));
	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--ip")==0)
		{
			if (flags[0]|| i >= argc - 1)
				return -1;
			flags[0] = 1;
			strcpy(ip, argv[i + 1]);
		}
		if (strcmp(argv[i], "--port") == 0)
		{
			if (flags[1] || i >= argc - 1)
				return -1;
			flags[1] = 1;
			*port = atoi(argv[i + 1]);
		}
		if (strcmp(argv[i], "--block") == 0 || strcmp(argv[i], "--nonblock") == 0)
		{
			if (flags[2])
				return -1;
			if (strcmp(argv[i], "--block") == 0)
				*isblock = 1;
			else
				*isblock = 0;
			flags[2] = 1;
		}
		if (strcmp(argv[i], "--fork") == 0 || strcmp(argv[i], "--nofork") == 0)
		{
			if (flags[3])
				return -1;
			if (strcmp(argv[i], "--fork") == 0)
				*isfork = 1;
			else
				*isfork = 0;
			flags[3] = 1;
		}
	}
	
	for (i = 0; i < 4; i++)
	{
		if (i == 1)
		{
			if (flags[i] == 0)
				return -1;
		}
		else
		{
			if (flags[i] == 0)
			{
				switch (i)
				{
				case 0:
					strcpy(ip,"0.0.0.0");
					break;
				case 2:
					*isblock = 0;
					break;
				case 3:
					*isfork = 0;
					break;
				default:
					break;
				}
			}
		}
	}
	
	if (*isfork == 0 && *isblock == 1)    //--nofork 与--block 同时出现时，--block 无效
		*isblock = 0;
	return 0;
}

int main(int argc,const char * argv[])
{
/*	if(argc!=2)
	{
		printf("Usage:%s [loacl_port]\n",argv[0]);
		return 1;
	}*/
	int isblock=0;//缺省为0
	int isfork=0;//缺省为0
	char ip[20]="0.0.0.0";//缺省为0.0.0.0
	int port;//无缺省值
	if(get_parameter(argc,argv,ip,&port,&isblock,&isfork)==-1){
		printf("参数输入有误！！!\n");
		return 0;
	}
	
	system("mkdir txt");
	
	
	if(isblock==1&&isfork==1){//阻塞+fork子程序
		fork_block_work(port,ip);
	}
	else if(isblock==1&&isfork==0){//阻塞+不fork子程序，此时阻塞失效，改为非阻塞
		nofork_work(port,ip);
	}
	else if(isblock==0&&isfork==1){//非阻塞+fork子程序
		fork_work(port,ip);
	}
	else if(isblock==0&&isfork==0){//非阻塞+不fork子程序
		nofork_work(port,ip);
	}


	return 0;
}
