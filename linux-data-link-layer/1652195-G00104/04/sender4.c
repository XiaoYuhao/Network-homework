#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include<sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>
#include "../common/tools.h"

#define MAX_SEQ 1
#define SIG_SPL_SHOULD_READ SIGRTMIN
#define SIG_SPL_ACK_REACH SIGRTMIN+2
#define SIG_SPL_CHSUM_ERR SIGRTMIN+1
#define SIG_SNL_SHOULD_READ SIGRTMIN+3
#define MAX_NETWORK_SHARE 1000
#define inc(k) if (k < MAX_SEQ) k = k + 1; else k = 0

const char *source_file1 = "send_test.txt";
const char *source_file2 = "recv_test.txt";
clock_t timer[MAX_SEQ];
pid_t spl_pid, sdl_pid, snl_pid;
int spl_ack_flag=0;
int spl_cksum_err=0;
int spl_should_read = 0;
int snl_should_read=0;
struct pidd{
	pid_t spl_pidd;
	pid_t sdl_pidd;
	pid_t snl_pidd;
};

void sigroutine(int sig)
{
	if (sig == SIG_SPL_SHOULD_READ)
	{
		spl_should_read = 1;
		//printf("spl_should_read sig has been recv\n");
	}
	if(sig==SIG_SPL_ACK_REACH){
		spl_ack_flag=1;
		printf("ack has been recv\n");
	}
	if(sig==SIG_SPL_CHSUM_ERR){
		spl_cksum_err=1;
		printf("err has been recv\n");
	}
	if(sig==SIG_SNL_SHOULD_READ){
		snl_should_read=1;//网络层需要接受数据
	}
	
}

void start_timer(seq_nr k){
	timer[k%MAX_SEQ]=clock();
}

void stop_timer(seq_nr k){
	timer[k%MAX_SEQ]=0;
}

void wait_for_event(event_type *event)
{
	if (signal(SIG_SPL_ACK_REACH, sigroutine) < 0)
	{ //装载信号
		perror("signal");
		exit(1);
	}
	if (signal(SIG_SPL_CHSUM_ERR, sigroutine) < 0)
	{ //装载信号
		perror("signal");
		exit(1);
	}
	int i;
	while (1)
	{
		clock_t t=clock();
		for(i=0;i<MAX_SEQ;i++){
			if(timer[i]!=0&&t-timer[i]>3000){//超时事件3秒
				*event=timeout;
				return ;
			}
		}
		if (spl_ack_flag == 1)
		{
			*event = frame_arrival;
			spl_ack_flag = 0;
			return;
		}
		if	(spl_cksum_err	==	1)
		{
			*event=cksum_err;
			spl_cksum_err=0;
			return;
		}
		else{
			usleep(1000 * 50);
		}
	}
}


void SNL()
{
	printf("SNL is running\n");
	struct pidd piddt;
	read_share_file2((void*)&piddt,"pid_no",-1,sizeof(struct pidd));
	spl_pid=piddt.spl_pidd;
	sdl_pid=piddt.sdl_pidd;
	snl_pid=piddt.snl_pidd;
	printf("SNL spl_pid:%d sdl_pid:%d snl_pid:%d\n",spl_pid,sdl_pid,snl_pid);
	
	if (signal(SIG_SNL_SHOULD_READ, sigroutine) < 0)
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
		perror_exit("fopen");
	
	while (1)
	{
		//printf("RNL is waiting for rdl_flag...\n");
		if (snl_should_read == 1)
		{
			char recvbuf[1024];
			read_share_file(recvbuf, "sdl_to_snl", -1, 1024);
			//printf("recvbuf:%s\n",recvbuf);
			FILE *fp2 = fopen(source_file2, "ab");
			if (fp2 == NULL)
			{
				perror_exit("fopen");
			}
			fwrite(recvbuf, 1, 1024, fp2);
			printf("SNL has write %d packet\n",++sum);
			snl_should_read = 0;
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
				usleep(1000*50);
			}
		}
	}
}

void to_network_layer(packet *p)
{
	char sendbuf[1024];
	memcpy(sendbuf, p, 1024);
	//printf("sendbuf:%s\n",sendbuf);
	write_share_file(sendbuf, "sdl_to_snl", -1, 1024); //写共享文件
	kill(snl_pid, SIG_SNL_SHOULD_READ);//向网络层发信号，读取数据
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

void to_physical_layer(frame *s)
{
	char buffer[1036];
	//转成网络序
	s->ack=htonl(s->ack);
	s->seq=htonl(s->seq);
	memcpy(buffer, s, 1036);
	write_share_file(buffer, "sdl_to_spl_pkg.dat", -1, 1036);
	kill(spl_pid, SIG_SPL_SHOULD_READ);
}

void from_physical_layer(frame *s)
{
	char recvbuf[1036];
	read_share_file(recvbuf, "spl_to_sdl", -1, 1036);
	memcpy(s, recvbuf, 1036);
	//转成主机序
	s->ack=ntohl(s->ack);
	s->seq=ntohl(s->seq);
}


void SDL()
{
	printf("SDL is running\n");
	struct pidd piddt;
	read_share_file2((void*)&piddt,"pid_no",-1,sizeof(struct pidd));
	spl_pid=piddt.spl_pidd;
	sdl_pid=piddt.sdl_pidd;
	snl_pid=piddt.snl_pidd;
	printf("SDL spl_pid:%d sdl_pid:%d snl_pid:%d\n",spl_pid,sdl_pid,snl_pid);

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
		memcpy(&s.info,buffer.data,1024);
		s.seq=next_frame_to_send;
		s.ack=1-frame_expected;
		to_physical_layer(&s);
		start_timer(s.seq);
	}
}


void SPL(int server_port, const char *server_ip)
{
	printf("SPL is running\n");
	struct pidd piddt;
	read_share_file2((void*)&piddt,"pid_no",-1,sizeof(struct pidd));
	spl_pid=piddt.spl_pidd;
	sdl_pid=piddt.sdl_pidd;
	snl_pid=piddt.snl_pidd;
	printf("SPL spl_pid:%d sdl_pid:%d snl_pid:%d\n",spl_pid,sdl_pid,snl_pid);
	int socket = init_sender_socket(server_port, server_ip);
	
	
	if (signal(SIG_SPL_SHOULD_READ, sigroutine) < 0)
	{
		perror_exit("signal");
	}
	int sum=0;

	int val;
	if(val=fcntl(socket,F_GETFL,0)<0){//获取文件状态标志
		perror_exit("fcntl");
		close(socket);
		exit(1);
	}
	if(fcntl(socket,F_SETFL,val|O_NONBLOCK)<0){//设置文件状态标志
		perror_exit("fcntl");
		close(socket);
		exit(1);
	}
	fd_set rfd,wfd;
				
	while (true)
	{
		
	/*	if (spl_should_read == true)
		{
			char *buffer;
			read_share_file(buffer, "sdl_to_spl_pkg.dat", -1, 1036);

			sum++;
			printf("send_num:%d\n",sum);
			write(socket, buffer, 1036);
			spl_should_read = false;
		}
		else{
			usleep(1000*1000);
			printf("wait for data from SDL\n");
		}
	*/
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_SET(socket,&rfd);
		FD_SET(socket,&wfd);	
		if(select(socket+1,&rfd,&wfd,NULL,0)>0){
			if(FD_ISSET(socket,&wfd)){
				FD_CLR(socket,&wfd);	
				if (spl_should_read == 1)
				{	
					char buffer[1036];
					read_share_file(buffer, "sdl_to_spl_pkg.dat", -1, 1036);
					sum++;
					printf("send_num:%d\n",sum);
					write(socket, buffer, 1036);
					spl_should_read = 0;
				}
				else{
					usleep(1000*50);
					//printf("wait for data from SDL\n");
				}
			}	
			if(FD_ISSET(socket,&rfd)){
				FD_CLR(socket,&rfd);		
				char buffer[1036];
				int ret=read(socket,buffer,sizeof(buffer));
				if(ret<=0){
					perror("read");
					exit(2);
				}
				int t=rand()%100;
				if(t<3)kill(sdl_pid,SIG_SPL_CHSUM_ERR);//3%的概率发cksum_err
				else if(t>96){}//3%的概率ack包丢失.啥也不干
				else {//向链路层发送ACK到达信号
					write_share_file(buffer, "spl_to_sdl", -1, 1036); 
					kill(sdl_pid,SIG_SPL_ACK_REACH);
				}
				
			}
			
		}
	}
}

int main(int argc, const char *argv[])
{
	int server_port = atoi(argv[1]);
	const char *server_ip = argv[2];
	
	delete_share_bool_memory(1234);

	snl_pid = fork();
	if (snl_pid < 0)
	{
		perror_exit("fork");
		exit(EXIT_FAILURE);
	}
	else if (snl_pid == 0)
	{
		sleep(1);
		SNL();
		printf("SNL is exit\n");
		exit(EXIT_SUCCESS);
	}
	spl_pid = fork();

	if (spl_pid < 0)
	{
		perror_exit("fork");
		exit(EXIT_FAILURE);
	}
	else if (spl_pid == 0)
	{
		sleep(2);
		SPL(server_port, server_ip);
		printf("SPL is exit\n");
		exit(EXIT_SUCCESS);
	}

	sdl_pid = fork();
	if (sdl_pid < 0)
	{
		perror_exit("fork");
		exit(EXIT_FAILURE);
	}
	else if (sdl_pid == 0)
	{
		sleep(3);
		SDL();
		printf("SDL is exit\n");
		exit(EXIT_SUCCESS);
	}
	
	struct pidd piddt;
	
	piddt.spl_pidd=spl_pid;//通过共享文件方式向子进程通知各个子进程的pid，使之之后能使用信号通信
	piddt.sdl_pidd=sdl_pid;
	piddt.snl_pidd=snl_pid;
	
	write_share_file2((void*)&piddt,"pid_no",-1,sizeof(struct pidd));
	
	int status;
	while(1){
		pid_t p=waitpid(-1,&status,0);
		printf("%d 进程exit:%d\n",p,status);
	}
	
	return 0;
}