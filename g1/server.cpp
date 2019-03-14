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
	
	printf("�����˿ڣ�%d\n",port);
	
	while(1){
		int conn=-1;
		conn=accept(listen_sock,(struct sockaddr*)&remote,&len);//��������
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
			if(pid==0){//�ӽ���
				if(val=fcntl(conn,F_GETFL,0)<0){//��ȡ�ļ�״̬��־
					perror("fcntl");
					close(conn);
					return -1;
				}
				if(fcntl(conn,F_SETFL,val|O_NONBLOCK)<0){//�����ļ�״̬��־
					perror("fcntl");
					close(conn);
					return -1;
				}
				int work=0,ret;
				Client_data client_data;//��Ŵ�client���յ�������
				int client_fd;
				char oldfname[MAXLEN];
				sprintf(oldfname,"%d.txt",conn);
				client_fd=open(oldfdname,O_CREATE|O_RDWR,0777);
				while(1){
					if(work==1){//����client���͵�ѧ��
						ret=read(conn,client_data.stuno,sizeof(client_data.stuno));
						if(ret<=0){
							perror("client%d read",i);
							return 0;
						}
						work=2;
					}
					else if(work==3){//����client���͵�pid
						ret=read(conn,client_data.pid,sizeof(client_data.pid));
						if(ret<=0){
							perror("client%d read",i);
							return 0;
						}
						work=4;
					}
					else if(work==5){//����client���͵�ϵͳʱ��
						ret=read(conn,client_data.time,sizeof(client_data.time));
						if(ret<=0){
							perror("client%d read",i);
							return 0;
						}
						work=6;
					}
					else if(work==7){//����client���͵�����ַ���
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
						if(ret==0){//read�յ�0��ʾclient���Ѿ��Ͽ�			
							char newfname[MAXLEN];
							sfprintf(newfdname,"%s.%s.pid.txt",client.stuno,client.pid);
							if(rename(oldfname,newfname)!=0){//����
								perror("rename");
							}
							char wbuf[MAXLEN*4];
							sfprintf(wbuf,"%s\n%s\n%s\n%s\n",client.stuno,client.pid,client.time,client.num);
							write(client_fd,wbuf,sizeof(wbuf));//д���ļ�
							close(conn);
							exit(1);//�˳��ӽ���
						}
						else{
									//������
						}
					}
					if(work==0){
						char sendbuf[MAXLEN]="StuNo";//����StuNo
						write(conn,sendbuf,sizeof(sendbuf));
						work=1;
					}
					else if(work==2){
						char sendbuf[MAXLEN]="pid";//����pid
						write(conn,sendbuf,sizeof(sendbuf));
						work=3;
					}
					else if(work==4){
						char sendbuf[MAXLEN]="TIME";//����TIME
						write(conn,sendbuf,sizeof(sendbuf));
						work=5;
					}
					else if(work==6){
						int num=rand()%100000;
						if(num<32768)num+=32768;
						char sendbuf[MAXLEN];
						sprintf(sendbuf,"str%d\n",num);//���������
						write(conn,sendbuf,sizeof(sendbuf));
						work=7;
					}
					else if(work==8){
						char sendbuf[MAXLEN]="end";//����end
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
	
	printf("�����˿ڣ�%d\n",port);
	
	fd_set rfd;
	while(1){
		FD_ZERO(&rfd);
		FD_SET(listen_sock,&rfd);
		
		int conn=-1;
		if(select(listen)sock+1,&rfd,NULL,NULL,0)>0){//���ܳ���һ����������
			conn=accept(listen_sock,(struct sockaddr*)&remote,&len);//��������
		}
		
		if(conn>0){
			pid_t pid;
			pid=fork();
			if(pid<0){
				perror("fork");
				return -1;
			}
			if(pid==0){//�ӽ���
				if(val=fcntl(conn,F_GETFL,0)<0){//��ȡ�ļ�״̬��־
					perror("fcntl");
					close(conn);
					return -1;
				}
				if(fcntl(conn,F_SETFL,val|O_NONBLOCK)<0){//�����ļ�״̬��־
					perror("fcntl");
					close(conn);
					return -1;
				}
				fd_set rfds,wfds;
				int work=0,ret;
				Client_data client_data;//��Ŵ�client���յ�������
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
							if(work==1){//����client���͵�ѧ��
								ret=read(conn,client_data.stuno,sizeof(client_data.stuno));
								if(ret<=0){
									perror("client%d read",i);
									return 0;
								}
								work=2;
							}
							else if(work==3){//����client���͵�pid
								ret=read(conn,client_data.pid,sizeof(client_data.pid));
								if(ret<=0){
									perror("client%d read",i);
									return 0;
								}
								work=4;
							}
							else if(work==5){//����client���͵�ϵͳʱ��
								ret=read(conn,client_data.time,sizeof(client_data.time));
								if(ret<=0){
									perror("client%d read",i);
									return 0;
								}
								work=6;
							}
							else if(work==7){//����client���͵�����ַ���
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
								if(ret==0){//read�յ�0��ʾclient���Ѿ��Ͽ�			
									char newfname[MAXLEN];
									sfprintf(newfdname,"%s.%s.pid.txt",client.stuno,client.pid);
									if(rename(oldfname,newfname)!=0){//����
										perror("rename");
									}
									char wbuf[MAXLEN*4];
									sfprintf(wbuf,"%s\n%s\n%s\n%s\n",client.stuno,client.pid,client.time,client.num);
									write(client_fd,wbuf,sizeof(wbuf));//д���ļ�
									close(conn);
									exit(1);//�˳��ӽ���
								}
								else{
									//������
								}
							}
						}
						if(FD_ISSET(conn,&wfds)){
							FD_CLR(conn,&wfds);
							if(work==0){
								char sendbuf[MAXLEN]="StuNo";//����StuNo
								write(conn,sendbuf,sizeof(sendbuf));
								work=1;
							}
							else if(work==2){
								char sendbuf[MAXLEN]="pid";//����pid
								write(conn,sendbuf,sizeof(sendbuf));
								work=3;
							}
							else if(work==4){
								char sendbuf[MAXLEN]="TIME";//����TIME
								write(conn,sendbuf,sizeof(sendbuf));
								work=5;
							}
							else if(work==6){
								int num=rand()%100000;
								if(num<32768)num+=32768;
								char sendbuf[MAXLEN];
								sprintf(sendbuf,"str%d\n",num);//���������
								write(conn,sendbuf,sizeof(sendbuf));
								work=7;
							}
							else if(work==8){
								char sendbuf[MAXLEN]="end";//����end
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
	int client[MAX]; //client�˼���
	int client_work[MAX];
	int client_fd[MAX];
	Client_data client_data[MAX];//��¼��client���յ�������
	int ret;
//	fd_set client_rfds[MAX]; //client���׽��ּ���
//	fd_set client_wfds[MAX];
	int max_no=0;	//��ǰclient����������
	
	int listen_sock=startup_noblock(port,ip);
	
	struct sockaddr_in remote;
	socklen_t len=sizeof(struct sockaddr_in);
	
	printf("�����˿ڣ�%d\n",port);
	
	fd_set rfd;
	fd_set client_rfds,client_wfds;
	int i,max_fd=0;
	for(i=0;i<MAX;i++){//��ʼ��client����
		client[i]=0;
		client_work[i]=0;
		client_fd[i]=0;
	}
	
	while(1){
		FD_ZERO(&rfd);
		FD_SET(listen_sock,&rfd);
		if(listen_sock>max_fd) max_fd=listen_sock;
		
		int conn=-1;
		if(select(max_fd+1,&rfd,NULL,NULL,0)>0){//���ܳ���һ����������
			conn=accept(listen_sock,(struct sockaddr*)&remote,&len);//��������
		}
		if(conn>0){//�ɹ�����
			for(i=0;i<MAX;i++){
				if(!client[i]){
					client[i]=conn;//�����ӳɹ���conn����client������
					if(client[i]>max_fd) max_fd=client[i];//����max_fd
					int val;
					if(val=fcntl(client[i],F_GETFL,0)<0){//��ȡ�ļ�״̬��־
						perror("fcntl");
						close(client[i]);
						return 0;
					}
					if(fcntl(client[i],F_SETFL,val|O_NONBLOCK)<0){//�����ļ�״̬��־
						perror("fcntl");
						close(client[i]);
						return 0;
					}
					max_no=i+1;
					printf("��%d��client�Ѿ�����\n",max_no);
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
			if(i==max_no)printf("��ǰclient������Ϊ%d\n",max_no);
		}
		if(select(max_fd+1,&client_rfds,&client_wfds,NULL,0)>0){
			for(int i=0;i<MAX;i++){
				if(client[i]){
					if(FD_ISSET(client[i],&rfds)){//client[i]�пɶ�������
						FD_CLR(client[i],&rfds);
						if(client_work[i]==1){//����client���͵�ѧ��
							ret=read(client[i],client_data[i].stuno,sizeof(client_data[i].stuno));
							if(ret<=0){
								perror("client%d read",i);
								return 0;
							}
							client_work[i]=2;
						}
						else if(client_work[i]==3){//����client���͵�pid
							ret=read(client[i],client_data[i].pid,sizeof(client_data[i].pid));
							if(ret<=0){
								perror("client%d read",i);
								return 0;
							}
							client_work[i]=4;
						}
						else if(client_work[i]==5){//����client���͵�ϵͳʱ��
							ret=read(client[i],client_data[i].time,sizeof(client_data[i].time));
							if(ret<=0){
								perror("client%d read",i);
								return 0;
							}
							client_work[i]=6;
						}
						else if(client_work[i]==7){//����client���͵�����ַ���
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
							if(ret==0){//read�յ�0��ʾclient���Ѿ��Ͽ�			
								char oldfname[MAXLEN];
								char newfname[MAXLEN];
								sfprintf(newfdname,"%s.%s.pid.txt",client_data[i].stuno,client_data[i].pid);
								sfprintf(oldfdname,"%d",client[i]);
								if(rename(oldfname,newfname)!=0){//����
									perror("rename");
									return 0;
								}
								char wbuf[MAXLEN*4];
								sfprintf(wbuf,"%s\n%s\n%s\n%s\n",client_data[i].stuno,client_data[i].pid,client_data[i].time,client_data[i].num);
								write(client_fd[i],wbuf,sizeof(wbuf));//д���ļ�
								close(client[i]);
							}
							else{
								//������
							}
						}
						
					}
					if(FD_ISSET(client[i],&wfds)){//client[i]��д
						FD_CLR(client[i],&wfds);
						if(client_work[i]==0){
							char sendbuf[MAXLEN]="StuNo";//����StuNo
							write(client[i],sendbuf,sizeof(sendbuf));
							client_work[i]=1;
						}
						else if(client_work[i]==2){
							char sendbuf[MAXLEN]="pid";//����pid
							write(client[i],sendbuf,sizeof(sendbuf));
							client_work[i]=3;
						}
						else if(client_work[i]==4){
							char sendbuf[MAXLEN]="TIME";//����TIME
							write(client[i],sendbuf,sizeof(sendbuf));
							client_work[i]=5;
						}
						else if(client_work[i]==6){
							int num=rand()%100000;
							if(num<32768)num+=32768;
							char sendbuf[MAXLEN];
							sprintf(sendbuf,"str%d\n",num);//���������
							write(client[i],sendbuf,sizeof(sendbuf));
							client_work[i]=7;
						}
						else if(client_work[i]==8){
							char sendbuf[MAXLEN]="end";//����end
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
	int isblock=0;//ȱʡΪ0
	int isfork=0;//ȱʡΪ0
	char ip[20];//ȱʡ0.0.0.0
	int port;//��ȱʡֵ
//	common(argv,isblock,isfork,ip,port);
	port=atoi(argv[1]);
	strcpy(ip,argv[2]);
	
	if(isblock==1&&isfork==1){//����+fork�ӳ���
		fork_block_work(port,ip);
	}
	else if(isblock==1&&isfork==0){//����+��fork�ӳ��򣬴�ʱ����ʧЧ����Ϊ������
		nofork_work(port,ip);
	}
	else if(isblock==0&&isfork==1){//������+fork�ӳ���
		fork_work(port,ip);
	}
	else if(isblock==0&&isfork==0){//������+��fork�ӳ���
		nofork_work(port,ip);
	}

	


	return 0;
}