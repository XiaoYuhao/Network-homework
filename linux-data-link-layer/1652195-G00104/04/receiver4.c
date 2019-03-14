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
#include<sys/wait.h>

#define MAX_NETWORK_SHARE 1000
#define MAX_SEQ 1
#define SIG_SPL_SHOULD_READ SIGRTMIN
#define SIG_RDL_SHOULD_READ SIGRTMIN+7
#define SIG_RNL_SHOULD_READ SIGRTMIN+8
#define SIG_RPL_SHOULD_READ SIGRTMIN+9
#define SIG_SPL_CHSUM_ERR SIGRTMIN+1
#define inc(k) if (k < MAX_SEQ) k = k + 1; else k = 0
const char *source_file1 = "send_test.txt";
const char *source_file2 = "recv_test.txt";
int conn;//发送方socket
clock_t timer[MAX_SEQ];
int rpl_flag = 0;
int rdl_flag = 0;
int rpl_cksum_err=0;
int rpl_send_flag=0;
pid_t rpl_pid, rdl_pid, rnl_pid;
bool rpl_should_read = false;
struct pidd{
	pid_t rpl_pidd;
	pid_t rdl_pidd;
	pid_t rnl_pidd;
};

void start_timer(seq_nr k){
	timer[k%MAX_SEQ]=clock();
}

void stop_timer(seq_nr k){
	timer[k%MAX_SEQ]=0;
}

void sigroutine(int sig)
{
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
		rpl_send_flag=1;
	}
	if(sig==SIG_SPL_CHSUM_ERR){
		int spl_cksum_err=1;
	}

}

void wait_for_event(event_type *event)
{
	if (signal(SIG_RDL_SHOULD_READ, sigroutine) < 0)
	{ //装载信号
		perror("signal");
		exit(1);
	}
	if (signal(SIG_SPL_CHSUM_ERR, sigroutine) < 0)
	{ //装载信号
		perror("signal");
		exit(1);
	}
	int i=0;
	while (1)
	{
		clock_t t=clock();
		for(i=0;i<MAX_SEQ;i++){
			if(timer[i]!=0&&t-timer[i]>3000){//超时事件3秒
				*event=timeout;
				return ;
			}
		}
		if (rpl_flag == 1)
		{
			*event = frame_arrival;
			rpl_flag = 0;
			return;
		}
		if(rpl_cksum_err==1){
			*event=cksum_err;
			rpl_cksum_err=0;
			return;
		}
		
		else{
			usleep(1000 * 50);
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
	
	if (signal(SIG_RNL_SHOULD_READ, sigroutine) < 0)
	{ //装载信号
		perror("signal");
		exit(1);
	}
	int sum=0;
	int share_file_pointer = 0;
	struct shared_bool_memory *shared = create_share_bool_memory(1234);
	size_t len;
	packet buffer;
	FILE *fp1 = fopen(source_file1, "r");
	if (fp1 == NULL)
		perror_exit("fopen1");
	
	while (1)
	{
		//printf("RNL is waiting for rdl_flag...\n");
		if (rdl_flag == 1)
		{
			char recvbuf[1024];
			read_share_file(recvbuf, "rdl_to_rnl", -1, 1024);
			//printf("recvbuf:%s\n",recvbuf);
			FILE *fp2 = fopen(source_file2, "ab");
			if (fp2 == NULL)
			{
				perror_exit("fopen2");
			}
			fwrite(recvbuf, 1, 1024, fp2);
			printf("RNL has write %d packet\n",++sum);
			rdl_flag = 0;
			fclose(fp2);
		}
		else{
			//usleep(1000 * 50);
			if(len = fread(buffer.data, 1, MAX_PKT, fp1)){
				while(shared->written1==1){
					usleep(1000*50);
					printf("shared->written=1");
				}
				//printf("write:network_datalink.share.%d\n",share_file_pointer);
				write_share_file(buffer.data, "network_datalink.share.", share_file_pointer, MAX_PKT);
				shared->written2=1;
				shared->text[share_file_pointer] = 1;
				shared->written2=0;
				//printf("%d\n",share_file_pointer);
				share_file_pointer = (++share_file_pointer) % MAX_NETWORK_SHARE;
			}
			else{
				usleep(1000 * 50);
			}
		}
	}
}

void from_network_layer(packet *buffer)
{
	struct shared_bool_memory* shared = get_share_bool_memory(1234);

	static int first_readable = 0;
	while (shared->text[first_readable] == 0){
		usleep(1000*50);
		first_readable = (++first_readable) % MAX_NETWORK_SHARE;
	//	printf("???????\n");
	}
	while(shared->written2==1){
		usleep(1000*50);
		printf("shared->written=1");
	}
	shared->written1=1;
	shared->text[first_readable]=0;
	shared->written1=0;
	//printf("read:network_datalink.share.%d\n",first_readable);
	read_share_file(buffer->data, "network_datalink.share.", first_readable, MAX_PKT);
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
	
	seq_nr next_frame_to_send;
	seq_nr frame_expected;
	frame r,s;
	packet buffer;
	event_type event;
	
	next_frame_to_send=0;
	frame_expected=0;
	from_network_layer(&buffer);
	//s.info=buffer;
	memcpy(&s.info,&buffer.data,1024);
	s.seq=next_frame_to_send;
	s.ack=1-frame_expected;
	to_physical_layer(&s);
	start_timer(s.seq);
	
	while(true){
		wait_for_event(&event);
		if(event==frame_arrival){
			from_physical_layer(&r);
			if(r.seq==frame_expected){
				to_network_layer(&r.info);
				inc(frame_expected);
			}
			if(r.ack==next_frame_to_send){
				stop_timer(r.ack);
				from_network_layer(&buffer);
				inc(next_frame_to_send);
			}
		}
		//s.info=buffer;
		memcpy(&s.info,&buffer.data,1024);
		s.seq=next_frame_to_send;
		s.ack=1-frame_expected;
		to_physical_layer(&s);
		start_timer(s.seq);
	}
}

void from_physical_layer(frame *s)
{
	char recvbuf[1036];
	read_share_file(recvbuf, "rpl_to_rdl", -1, 1036);
	memcpy(s, recvbuf, 1036);
	//转成主机序
	s->ack=ntohl(s->ack);
	s->seq=ntohl(s->seq);
}

void to_physical_layer(frame *s)
{
	char buffer[1036];
	//转成网络序
	s->ack=htonl(s->ack);
	s->seq=htonl(s->seq);
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
	
	int val;
	if(val=fcntl(conn,F_GETFL,0)<0){//获取文件状态标志
		perror("fcntl");
		close(conn);
		exit(1);
	}
	if(fcntl(conn,F_SETFL,val|O_NONBLOCK)<0){//设置文件状态标志
		perror("fcntl");
		close(conn);
		exit(1);
	}
	fd_set rfd,wfd;
	while (1)
	{
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_SET(conn,&rfd);
		FD_SET(conn,&wfd);
		if(select(conn+1,&rfd,&wfd,NULL,0)>0){
			if(FD_ISSET(conn,&wfd)){//可写
				FD_CLR(conn,&wfd);
				if(rpl_send_flag==1){
					char buffer[1036];
					read_share_file(buffer, "rdl_to_rpl_pkg.dat", -1, 1036);
					write(conn, buffer, 1036);
					printf("send a ack frame\n");
					rpl_send_flag=0;
				}
				else{
					usleep(1000*50);
				}
			}
			if(FD_ISSET(conn,&rfd)){//可读
				FD_CLR(conn,&rfd);
				char recvbuf[1036];
				int ret=read(conn, recvbuf, 1036);	 
				if(ret<=0){
					perror("read");
					exit(1);
				}
				int t=rand()%100;
				if(t<3)kill(rdl_pid, SIG_SPL_CHSUM_ERR);
				else if(t>96){}
				else {
					write_share_file(recvbuf, "rpl_to_rdl", -1, 1036); //收到数据后，写共享文件
					kill(rdl_pid, SIG_RDL_SHOULD_READ);//向链路层发信号，通知接受数据
				}
			}
		}
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
		sleep(3);
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
		sleep(2);
		RPL(port, argv[2]);
	}
	
	struct pidd piddt;
	
	piddt.rpl_pidd=rpl_pid;//通过共享文件方式向子进程通知各个子进程的pid，使之之后能使用信号通信
	piddt.rdl_pidd=rdl_pid;
	piddt.rnl_pidd=rnl_pid;
	
	write_share_file2((void*)&piddt,"pid_no",-1,sizeof(struct pidd));
	
	int status;
	while(1){
		pid_t p=waitpid(-1,&status,0);
		printf("%d 进程exit:%d\n",p,status);
	}

	return 0;
}