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
#include<random.h>
#define MAX 1024
#define MAXLEN 128

struct Client_data{
	char stuno[MAXLEN];
	char pid[MAXLEN];
	char time[MAXLEN];
	char num[MAXLEN];
};

int startup_block(int _port,const char *ip){
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

int startup_noblock(int _port,const char *ip)
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
	
	printf("监听端口：%d\n",port);
	
	while(1){
		int conn=-1;
		conn=accept(listen_sock,(struct sockaddr*)&remote,&len);//接受连接
		if(conn<0){
			perror("accept");
			return -1;
		}
		if(conn>0){
			pid_t pid;
			pid=fork();
			if(pid<0){
				perror("fork");
				return -1;
			}
			if(pid==0){//子进程
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
				int work=0,ret;
				Client_data client_data;//存放从client端收到的数据
				int client_fd;
				char oldfname[MAXLEN];
				sprintf(oldfname,"%d.txt",conn);
				client_fd=open(oldfdname,O_CREATE|O_RDWR,0777);
				while(1){
					if(work==1){//接收client发送的学号
						ret=read(conn,client_data.stuno,sizeof(client_data.stuno));
						if(ret<=0){
							perror("client%d read",i);
							return 0;
						}
						work=2;
					}
					else if(work==3){//接收client发送的pid
						ret=read(conn,client_data.pid,sizeof(client_data.pid));
						if(ret<=0){
							perror("client%d read",i);
							return 0;
						}
						work=4;
					}
					else if(work==5){//接收client发送的系统时间
						ret=read(conn,client_data.time,sizeof(client_data.time));
						if(ret<=0){
							perror("client%d read",i);
							return 0;
						}
						work=6;
					}
					else if(work==7){//接收client发送的随机字符串
						ret=read(conn,client_data.num,sizeof(client_data.num));
						if(ret<=0){
							perror("client%d read",i);
							return 0;
						}
						work=8;
					}
					else if(work==9){
						char recvbuf[MAXLEN];
						ret=read(conn,recvbuf,sizeof(recvbuf));
						if(ret==0){//read收到0表示client端已经断开			
							char newfname[MAXLEN];
							sfprintf(newfdname,"%s.%s.pid.txt",client.stuno,client.pid);
							if(rename(oldfname,newfname)!=0){//改名
								perror("rename");
							}
							char wbuf[MAXLEN*4];
							sfprintf(wbuf,"%s\n%s\n%s\n%s\n",client.stuno,client.pid,client.time,client.num);
							write(client_fd,wbuf,sizeof(wbuf));//写入文件
							close(conn);
							exit(1);//退出子进程
						}
						else{
									//出错处理
						}
					}
					if(work==0){
						char sendbuf[MAXLEN]="StuNo";//发送StuNo
						write(conn,sendbuf,sizeof(sendbuf));
						work=1;
					}
					else if(work==2){
						char sendbuf[MAXLEN]="pid";//发送pid
						write(conn,sendbuf,sizeof(sendbuf));
						work=3;
					}
					else if(work==4){
						char sendbuf[MAXLEN]="TIME";//发送TIME
						write(conn,sendbuf,sizeof(sendbuf));
						work=5;
					}
					else if(work==6){
						int num=rand()%100000;
						if(num<32768)num+=32768;
						char sendbuf[MAXLEN];
						sprintf(sendbuf,"str%d\n",num);//发送随机数
						write(conn,sendbuf,sizeof(sendbuf));
						work=7;
					}
					else if(work==8){
						char sendbuf[MAXLEN]="end";//发送end
						write(conn,sendbuf,sizeof(sendbuf));
						work=9;
					}
				}
			
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
		if(select(listen)sock+1,&rfd,NULL,NULL,0)>0){//可能出现一个连接请求
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
				char oldfname[MAXLEN];
				sprintf(oldfname,"%d.txt",conn);
				client_fd=open(oldfdname,O_CREATE|O_RDWR,0777);
				while(1){
					FD_ZERO(rfds);
					FD_ZERO(wfds);
					FD_SET(conn,&rfds);
					FD_SET(conn,&wfds);
					if(select(conn+1,&rfds,&wfds,NULL,0)>0){
						if(FD_ISSET(conn,&rfds)){
							FD_CLR(conn,&rfds);
							if(work==1){//接收client发送的学号
								ret=read(conn,client_data.stuno,sizeof(client_data.stuno));
								if(ret<=0){
									perror("client%d read",i);
									return 0;
								}
								work=2;
							}
							else if(work==3){//接收client发送的pid
								ret=read(conn,client_data.pid,sizeof(client_data.pid));
								if(ret<=0){
									perror("client%d read",i);
									return 0;
								}
								work=4;
							}
							else if(work==5){//接收client发送的系统时间
								ret=read(conn,client_data.time,sizeof(client_data.time));
								if(ret<=0){
									perror("client%d read",i);
									return 0;
								}
								work=6;
							}
							else if(work==7){//接收client发送的随机字符串
								ret=read(conn,client_data.num,sizeof(client_data.num));
								if(ret<=0){
									perror("client%d read",i);
									return 0;
								}
								work=8;
							}
							else if(work==9){
								char recvbuf[MAXLEN];
								ret=read(conn,recvbuf,sizeof(recvbuf));
								if(ret==0){//read收到0表示client端已经断开			
									char newfname[MAXLEN];
									sfprintf(newfdname,"%s.%s.pid.txt",client.stuno,client.pid);
									if(rename(oldfname,newfname)!=0){//改名
										perror("rename");
									}
									char wbuf[MAXLEN*4];
									sfprintf(wbuf,"%s\n%s\n%s\n%s\n",client.stuno,client.pid,client.time,client.num);
									write(client_fd,wbuf,sizeof(wbuf));//写入文件
									close(conn);
									exit(1);//退出子进程
								}
								else{
									//出错处理
								}
							}
						}
						if(FD_ISSET(conn,&wfds)){
							FD_CLR(conn,&wfds);
							if(work==0){
								char sendbuf[MAXLEN]="StuNo";//发送StuNo
								write(conn,sendbuf,sizeof(sendbuf));
								work=1;
							}
							else if(work==2){
								char sendbuf[MAXLEN]="pid";//发送pid
								write(conn,sendbuf,sizeof(sendbuf));
								work=3;
							}
							else if(work==4){
								char sendbuf[MAXLEN]="TIME";//发送TIME
								write(conn,sendbuf,sizeof(sendbuf));
								work=5;
							}
							else if(work==6){
								int num=rand()%100000;
								if(num<32768)num+=32768;
								char sendbuf[MAXLEN];
								sprintf(sendbuf,"str%d\n",num);//发送随机数
								write(conn,sendbuf,sizeof(sendbuf));
								work=7;
							}
							else if(work==8){
								char sendbuf[MAXLEN]="end";//发送end
								write(conn,sendbuf,sizeof(sendbuf));
								work=9;
							}
						}
					}
				}
			}
			
		}
	}
	close(listen_sock);
	return 1;
}

void nofork_work(int port,const char *ip){
	int client[MAX]; //client端集合
	int client_work[MAX];
	int client_fd[MAX];
	Client_data client_data[MAX];//记录从client端收到的数据
	int ret;
//	fd_set client_rfds[MAX]; //client端套接字集合
//	fd_set client_wfds[MAX];
	int max_no=0;	//当前client端连接数量
	
	int listen_sock=startup_noblock(port,ip);
	
	struct sockaddr_in remote;
	socklen_t len=sizeof(struct sockaddr_in);
	
	printf("监听端口：%d\n",port);
	
	fd_set rfd;
	fd_set client_rfds,client_wfds;
	int i,max_fd=0;
	for(i=0;i<MAX;i++){//初始化client集合
		client[i]=0;
		client_work[i]=0;
		client_fd[i]=0;
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
					char fdname[20];
					sprintf(fdname,"%d",client[i]);
					client_fd[i]=open(fdname,O_CREATE|O_RDWR,0777);
					break;
				}
			}
		}
		FD_ZERO(&client_rfds);
		FD_ZERO(&client_wfds);
		for(i=0;i<MAX;i++){
			if(client[i]){
				FD_SET(client[i],&client_rfds);
				FD_SET(client[i],&client_wfds);
			}
			if(i==max_no)printf("当前client连接数为%d\n",max_no);
		}
		if(select(max_fd+1,&client_rfds,&client_wfds,NULL,0)>0){
			for(int i=0;i<MAX;i++){
				if(client[i]){
					if(FD_ISSET(client[i],&rfds)){//client[i]有可读的数据
						FD_CLR(client[i],&rfds);
						if(client_work[i]==1){//接收client发送的学号
							ret=read(client[i],client_data[i].stuno,sizeof(client_data[i].stuno));
							if(ret<=0){
								perror("client%d read",i);
								return 0;
							}
							client_work[i]=2;
						}
						else if(client_work[i]==3){//接收client发送的pid
							ret=read(client[i],client_data[i].pid,sizeof(client_data[i].pid));
							if(ret<=0){
								perror("client%d read",i);
								return 0;
							}
							client_work[i]=4;
						}
						else if(client_work[i]==5){//接收client发送的系统时间
							ret=read(client[i],client_data[i].time,sizeof(client_data[i].time));
							if(ret<=0){
								perror("client%d read",i);
								return 0;
							}
							client_work[i]=6;
						}
						else if(client_work[i]==7){//接收client发送的随机字符串
							ret=read(client[i],client_data[i].num,sizeof(client_data[i].num));
							if(ret<=0){
								perror("client%d read",i);
								return 0;
							}
							client_work[i]=8;
						}
						else if(client_work[i]==9){
							char recvbuf[MAXLEN];
							ret=read(client[i],recvbuf,sizeof(recvbuf));
							if(ret==0){//read收到0表示client端已经断开			
								char oldfname[MAXLEN];
								char newfname[MAXLEN];
								sfprintf(newfdname,"%s.%s.pid.txt",client_data[i].stuno,client_data[i].pid);
								sfprintf(oldfdname,"%d",client[i]);
								if(rename(oldfname,newfname)!=0){//改名
									perror("rename");
									return 0;
								}
								char wbuf[MAXLEN*4];
								sfprintf(wbuf,"%s\n%s\n%s\n%s\n",client_data[i].stuno,client_data[i].pid,client_data[i].time,client_data[i].num);
								write(client_fd[i],wbuf,sizeof(wbuf));//写入文件
								close(client[i]);
							}
							else{
								//出错处理
							}
						}
						
					}
					if(FD_ISSET(client[i],&wfds)){//client[i]可写
						FD_CLR(client[i],&wfds);
						if(client_work[i]==0){
							char sendbuf[MAXLEN]="StuNo";//发送StuNo
							write(client[i],sendbuf,sizeof(sendbuf));
							client_work[i]=1;
						}
						else if(client_work[i]==2){
							char sendbuf[MAXLEN]="pid";//发送pid
							write(client[i],sendbuf,sizeof(sendbuf));
							client_work[i]=3;
						}
						else if(client_work[i]==4){
							char sendbuf[MAXLEN]="TIME";//发送TIME
							write(client[i],sendbuf,sizeof(sendbuf));
							client_work[i]=5;
						}
						else if(client_work[i]==6){
							int num=rand()%100000;
							if(num<32768)num+=32768;
							char sendbuf[MAXLEN];
							sprintf(sendbuf,"str%d\n",num);//发送随机数
							write(client[i],sendbuf,sizeof(sendbuf));
							client_work[i]=7;
						}
						else if(client_work[i]==8){
							char sendbuf[MAXLEN]="end";//发送end
							write(client[i],sendbuf,sizeof(sendbuf));
							client_work[i]=9;
						}
					}
				}
			}				
		}
	}
	
	close(listen_sock);
	return 1;
}

int common(const char *para[],int *isblock,int *isfork,char *ip,int *port){
	
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
	char ip[20];//缺省0.0.0.0
	int port;//无缺省值
//	common(argv,isblock,isfork,ip,port);
	port=atoi(argv[1]);
	strcpy(ip,argv[2]);
	
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