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
const char *source_file = "recv_test.txt";


int rpl_flag = 0;
int rdl_flag = 0;
pid_t rpl_pid, rdl_pid, rnl_pid;


void sigroutine(int sig)
{
	if (sig >= SIGRTMIN && sig <= SIGRTMIN + 5)
	{
		//printf("Oh I receive a sig %d.\n", sig);
		if (sig == SIGRTMIN + 1)
		{
			rpl_flag = 1;
		}
		if (sig == SIGRTMIN + 2)
		{
			rdl_flag = 1;
		}
	}
}

void RNL()
{
	printf("RNL is running\n");
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
			int i;
			for(i=0;i<1024;i++){
				if(recvbuf[i]!='\0')break;
			}
			if(i==1023){
				printf("recv finished\n");
				break;
			}
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
	kill(rnl_pid, SIGRTMIN + 2);
}

void RDL(pid_t pid)
{
	printf("RDL is running\n");
	rnl_pid = pid;
	frame r;
	event_type event;
	printf("rnl_pid=%d\n",pid);

	while (true)
	{
		//printf("RDL is waiting event...\n");
		wait_for_event(&event);
		//printf("RDL has recv a event...\n");
		from_physical_layer(&r);
		to_network_layer(&r.info);
	}
}

void from_physical_layer(frame *s)
{
	char recvbuf[1036];
	read_share_file(recvbuf, "rpl_to_rdl", -1, 1036);
	memcpy(s, recvbuf, 1036);
}

void RPL(int port, const char *ip, pid_t pid)
{
	printf("RPL is running\n");
	rdl_pid = pid;
	printf("rdl_pid=%d\n",pid);
	int listen_sock = init_recv_socket(port, ip);
	struct sockaddr_in remote;
	socklen_t len=sizeof(struct sockaddr_in);
	int conn=accept(listen_sock,(struct sockaddr*)&remote,&len);
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
		//printf("read:%s\n",recvbuf);
		write_share_file(recvbuf, "rpl_to_rdl", -1, 1036); //收到数据后，写共享文件
		kill(rdl_pid, SIGRTMIN + 1);					   //发完数据后发送信号
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
		RDL(rnl_pid);
	}
	rpl_pid = fork();

	if (rpl_pid < 0)
	{
		perror("fork");
		exit(1);
	}
	else if (rpl_pid == 0)
	{
		RPL(port, argv[2], rdl_pid);
	}

	return 0;
}