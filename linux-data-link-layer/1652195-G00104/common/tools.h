#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>
#include "protocol.h"

#define MAX_NETWORK_SHARE 1000


void perror_exit(const char *hint)
{
    FILE *fp;
	fp=fopen("rsyslog","w");
    printf("Function [%s] failed: %s (errno: %d)\n",hint,strerror(errno),errno);
	char info[128];
	sprintf(info,"Function [%s] failed: %s (errno: %d)\n",hint,strerror(errno),errno);
	fwrite(info,1,sizeof(info),fp);
	fclose(fp);
    exit(1);    
}

void write_share_file(const char *buf, const char *file_name_head, int file_pointer, int size)
{
	char filename[50];
	strcpy(filename, file_name_head);

	if(file_pointer >= 0)
	{
		char str_file_pointer[10];
		//itoa(file_pointer, str_file_pointer, 10);
		sprintf(str_file_pointer,"%d",file_pointer);
		strcat(filename, str_file_pointer);
	}

	FILE *fp = fopen(filename, "w");
	if (fp == NULL)
		perror_exit("fopen");

	fwrite(buf, 1, size, fp);
	fclose(fp);
/*	int fd=open(filename,O_WRONLY|O_CREAT|O_TRUNC);
	if(fd<0){
		perror_exit("open");
	}
	int ret=write(fd,buf,size);
	if(ret<0){
		perror_exit("write");
	}
	close(fd);*/
}

void read_share_file(char *buf, const char *file_name_head, int file_pointer, int size)
{
	char filename[50];
	strcpy(filename, file_name_head);

	if(file_pointer >= 0)
	{
		char str_file_pointer[10];
		//itoa(file_pointer, str_file_pointer, 10);
		sprintf(str_file_pointer,"%d",file_pointer);
		strcat(filename, str_file_pointer);
	}

	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
		perror_exit("fopen");

	int ret=fread(buf, 1, size, fp);
	fclose(fp);
/*	int fd=open(filename,O_RDONLY|O_CREAT|O_TRUNC);
	if(fd<0){
		perror_exit("open");
	}
	int ret=read(fd,buf,size);
	if(ret<0){
		perror_exit("write");
	}
	close(fd);*/
}


int init_sender_socket(int server_port, const char *server_ip)
{
    //init socket
    int client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(client_sock==-1)
        perror_exit("socket");

    //set REUSEADDR
    int flag=1,flaglen=sizeof(int);
	

    setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &flag, flaglen);
    //connect
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port); 
	while(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr))==-1){
		printf("正在尝试连接,请确保接收方已经建立...\n");
		sleep(1);
	}

    printf("Connect to %s:%d success!\n",server_ip,server_port);
    return client_sock;
}

int init_recv_socket(int _port,const char *ip){
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
	
	int flag=1,flaglen=sizeof(int);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, flaglen);
	
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

struct shared_bool_memory
{
	bool written1;//作为一个标志，非0：表示可读，0表示可写
	bool written2;
	int text[MAX_NETWORK_SHARE];//记录写入和读取的文本
};

struct shared_bool_memory* create_share_bool_memory(int id)
{
	int shmid;

	if((shmid = shmget((key_t)id, sizeof(struct shared_bool_memory), 0666|IPC_CREAT)) < 0)
		perror_exit("shmget");

	void* shm;

	if((shm = shmat(shmid, 0, 0)) == (void*)-1)
		perror_exit("shmat");

	struct shared_bool_memory *shared = (struct shared_bool_memory*)shm;
/*	while (shared->written == true)
	{
		sleep(1);
		printf("暂时不能写入,等待中...\n");
	}*/

	// 将新建的共享内存与buffer绑定，此后修改buffer即修改了内存内容
	//shared->text = buffer;
	shared->written1=0;
	shared->written2=0;
	return shared;
}

struct shared_bool_memory* get_share_bool_memory(int id)
{
	int shmid;

	if((shmid = shmget((key_t)id, sizeof(struct shared_bool_memory), 0666|IPC_CREAT)) < 0)
		perror_exit("shmget");

	void* shm;

	if((shm = shmat(shmid, 0, 0)) == (void*)-1)
		perror_exit("shmat");

	struct shared_bool_memory *shared = (struct shared_bool_memory*)shm;
/*	while (shared->written == true)
	{
		sleep(1);
		printf("暂时不能写入,等待中...\n");
	}*/

	// 将buffer与共享内存绑定，此后修改buffer即修改了内存内容
	return shared;
}

void delete_share_bool_memory(int id)
{
	int shmid;

	if((shmid = shmget((key_t)id, sizeof(struct shared_bool_memory), 0666|IPC_CREAT)) < 0)
		perror_exit("shmget");

	void* shm;

	if((shm = shmat(shmid, 0, 0)) == (void*)-1)
		perror_exit("shmat");

	struct shared_bool_memory *shared = (struct shared_bool_memory*)shm;

	if(shmdt(shared)==-1){
		perror("shmdt");
		exit(1);
	}
	
	if(shmctl(shmid,IPC_RMID,0)==-1){
		perror("shmctl");
		exit(1);
	}
}

void write_share_file2(const void *buf, const char *file_name_head, int file_pointer, int size)
{
	char filename[50];
	strcpy(filename, file_name_head);

	if(file_pointer >= 0)
	{
		char str_file_pointer[10];
		//itoa(file_pointer, str_file_pointer, 10);
		sprintf(str_file_pointer,"%d",file_pointer);
		strcat(filename, str_file_pointer);
	}

	FILE *fp = fopen(filename, "w");
	if (fp == NULL)
		perror_exit("fopen");

	fwrite(buf, 1, size, fp);
	fclose(fp);
/*	int fd=open(filename,O_WRONLY|O_CREAT|O_TRUNC);
	if(fd<0){
		perror_exit("open");
	}
	int ret=write(fd,buf,size);
	if(ret<0){
		perror_exit("write");
	}
	close(fd);*/
}

void read_share_file2(void *buf, const char *file_name_head, int file_pointer, int size)
{
	char filename[50];
	strcpy(filename, file_name_head);

	if(file_pointer >= 0)
	{
		char str_file_pointer[10];
		//itoa(file_pointer, str_file_pointer, 10);
		sprintf(str_file_pointer,"%d",file_pointer);
		strcat(filename, str_file_pointer);
	}

	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
		perror_exit("fopen");

	fread(buf, 1, size, fp);
	fclose(fp);
/*	int fd=open(filename,O_RDONLY|O_CREAT|O_TRUNC);
	if(fd<0){
		perror_exit("open");
	}
	int ret=read(fd,buf,size);
	if(ret<0){
		perror_exit("write");
	}
	close(fd);*/
}