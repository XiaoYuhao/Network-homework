#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>
#include "../common/tools.h"

#define MAX_NETWORK_SHARE 1000
#define SIG_SPL_SHOULD_READ SIGRTMIN
#define SIG_RDL_SHOULD_READ SIGRTMIN+1
#define SIG_RNL_SHOULD_READ SIGRTMIN+2
#define SIG_RPL_SHOULD_READ SIGRTMIN+3
const char *source_file = "recv_test.txt";
int conn;//发送方socket

int rpl_flag = 0;
int rdl_flag = 0;
int rpl_send_flag=0;
pid_t rpl_pid, rdl_pid, rnl_pid;
bool rpl_should_read = false;
struct pidd{
	pid_t rpl_pidd;
	pid_t rdl_pidd;
	pid_t rnl_pidd;
};

void sigroutine(int sig)
{
	if (sig >= SIGRTMIN && sig <= SIGRTMIN + 5)
	{
		//printf("Oh I receive a sig %d.\n", sig);
		if (sig == SIG_RDL_SHOULD_READ)
		{
			rpl_flag = 1;
		}
		if (sig == SIG_RNL_SHOULD_READ)
		{
			rdl_flag = 1;
		}
		if (sig == SIG_RPL_SHOULD_READ)
		{
			//rpl_send_flag=1;
			char buffer[12];
			frame_check s;
			s.kind=ack;
			s.seq=0xffffffff;
			memcpy(buffer,&s,sizeof(s));
			int t=rand()%10;
			if(t<1)sleep(2);//模拟延时发ack
			write(conn, buffer, sizeof(buffer));
			printf("send a ack frame\n");
		}
	}
}
void RNL()
{
	printf("RNL is running\n");
	struct pidd piddt;
	read_share_file2((void*)&piddt,"pid_no",-1,sizeof(struct pidd));
	rpl_pid=piddt.rpl_pidd;
	rdl_pid=piddt.rdl_pidd;
	rnl_pid=piddt.rnl_pidd;
	printf("rpl_pid:%d pdl_pid:%d rnl_pid:%d\n",rpl_pid,rdl_pid,rnl_pid);
	
	if (signal(SIGRTMIN + 2, sigroutine) < 0)
	{ //装载信号
		perror("signal");
		exit(1);
	}
	int sum=0;
	while (1)
	{
		//printf("RNL is waiting for rdl_flag...\n");
		if (rdl_flag == 1)
		{
			char recvbuf[1024];
			read_share_file(recvbuf, "rdl_to_rnl", -1, 1024);
			//printf("recvbuf:%s\n",recvbuf);
			FILE *fp = fopen(source_file, "ab");
			if (fp == NULL)
			{
				perror_exit("fopen");
			}
			fwrite(recvbuf, 1, 1024, fp);
			printf("RNL has write %d packet\n",++sum);
			rdl_flag = 0;
			fclose(fp);
		}
		else{
			usleep(1000 * 50);
		}
	}
}

void wait_for_event(event_type *event)
{
	if (signal(SIGRTMIN + 1, sigroutine) < 0)
	{ //装载信号
		perror("signal");
		exit(1);
	}

	while (1)
	{
		if (rpl_flag == 1)
		{
			*event = frame_arrival;
			rpl_flag = 0;
			return;
		}
		else{
			usleep(1000 * 50);
		}
	}
}

void to_network_layer(packet *p)
{
	char sendbuf[1024];
	memcpy(sendbuf, p, 1024);
	//printf("sendbuf:%s\n",sendbuf);
	write_share_file(sendbuf, "rdl_to_rnl", -1, 1024); //写共享文件
	kill(rnl_pid, SIG_RNL_SHOULD_READ);//向网络层发信号，读取数据
}

void RDL()
{
	printf("RDL is running\n");
	struct pidd piddt;
	read_share_file2((void*)&piddt,"pid_no",-1,sizeof(struct pidd));
	rpl_pid=piddt.rpl_pidd;
	rdl_pid=piddt.rdl_pidd;
	rnl_pid=piddt.rnl_pidd;
	printf("rpl_pid:%d pdl_pid:%d rnl_pid:%d\n",rpl_pid,rdl_pid,rnl_pid);
	frame r,s;
	event_type event;

	while (true)
	{
		//printf("RDL is waiting event...\n");
		wait_for_event(&event);
		//printf("RDL has recv a event...\n");
		from_physical_layer(&r);
		to_network_layer(&r.info);
		to_physical_layer(&s);//发送ACK
	}
}

void from_physical_layer(frame *s)
{
	char recvbuf[1036];
	read_share_file(recvbuf, "rpl_to_rdl", -1, 1036);
	memcpy(s, recvbuf, 1036);
}

void to_physical_layer(frame *s)
{
	char buffer[1036];
	memcpy(buffer, s, 1036);
	write_share_file(buffer, "rdl_to_rpl_pkg.dat", -1, 1036);
	kill(rpl_pid, SIG_RPL_SHOULD_READ);//向物理层发信号，通知其发ACK帧
}

void RPL(int port, const char *ip)
{
	printf("RPL is running\n");
	struct pidd piddt;
	read_share_file2((void*)&piddt,"pid_no",-1,sizeof(struct pidd));
	rpl_pid=piddt.rpl_pidd;
	rdl_pid=piddt.rdl_pidd;
	rnl_pid=piddt.rnl_pidd;
	printf("rpl_pid:%d pdl_pid:%d rnl_pid:%d\n",rpl_pid,rdl_pid,rnl_pid);
	
	if (signal(SIG_RPL_SHOULD_READ, sigroutine) < 0)//装载信号
	{
		perror_exit("signal");
	}
	
	int listen_sock = init_recv_socket(port, ip);
	struct sockaddr_in remote;
	socklen_t len=sizeof(struct sockaddr_in);
	conn=accept(listen_sock,(struct sockaddr*)&remote,&len);
	if(conn<0){
		perror("conn");
		exit(1);
	}
	printf("Connect is success\n");
	while (1)
	{
		char recvbuf[1036];
		int ret=read(conn, recvbuf, sizeof(recvbuf));	   //没有数据会阻塞
		if(ret<=0){
			perror("read");
			exit(1);
		}
		write_share_file(recvbuf, "rpl_to_rdl", -1, 1036); //收到数据后，写共享文件
		kill(rdl_pid, SIG_RDL_SHOULD_READ);//向链路层发信号，通知接受数据
	}
}

int main(int argc, const char *argv[])
{
	int port = atoi(argv[1]);

	rnl_pid = fork();
	if (rnl_pid < 0)
	{
		perror("fork");
		exit(1);
	}
	else if (rnl_pid == 0)
	{
		sleep(1);
		RNL();
	}

	rdl_pid = fork();
	if (rdl_pid < 0)
	{
		perror("fork");
		exit(1);
	}
	else if (rdl_pid == 0)
	{
		sleep(1);
		RDL();
	}
	rpl_pid = fork();

	if (rpl_pid < 0)
	{
		perror("fork");
		exit(1);
	}
	else if (rpl_pid == 0)
	{
		sleep(1);
		RPL(port, argv[2]);
	}
	
	struct pidd piddt;
	
	piddt.rpl_pidd=rpl_pid;//通过共享文件方式向子进程通知各个子进程的pid，使之之后能使用信号通信
	piddt.rdl_pidd=rdl_pid;
	piddt.rnl_pidd=rnl_pid;
	
	write_share_file2((void*)&piddt,"pid_no",-1,sizeof(struct pidd));
	
	

	return 0;
}