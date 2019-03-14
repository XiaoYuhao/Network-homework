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
#define MAX_SEQ 1
#define SIG_SPL_SHOULD_READ SIGRTMIN
#define SIG_RDL_SHOULD_READ SIGRTMIN+7
#define SIG_RNL_SHOULD_READ SIGRTMIN+8
#define SIG_RPL_SHOULD_READ SIGRTMIN+9
#define SIG_SPL_CHSUM_ERR SIGRTMIN+1
#define inc(k) if (k < MAX_SEQ) k = k + 1; else k = 0

const char *source_file = "recv_test.txt";
int conn;//���ͷ�socket
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
	{ //װ���ź�
		perror("signal");
		exit(1);
	}
	if (signal(SIG_SPL_CHSUM_ERR, sigroutine) < 0)
	{ //װ���ź�
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
	{ //װ���ź�
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



void to_network_layer(packet *p)
{
	char sendbuf[1024];
	memcpy(sendbuf, p, 1024);
	//printf("sendbuf:%s\n",sendbuf);
	write_share_file(sendbuf, "rdl_to_rnl", -1, 1024); //д�����ļ�
	kill(rnl_pid, SIG_RNL_SHOULD_READ);//������㷢�źţ���ȡ����
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
	
	seq_nr frame_expected;
	frame r,s;
	event_type event;
	frame_expected=0;
	while(true){
		wait_for_event(&event);
		if(event==frame_arrival){
			from_physical_layer(&r);
			if(r.seq==frame_expected){//���ƥ��
				to_network_layer(&r.info);
				inc(frame_expected);
			}
			s.ack=1-frame_expected;//�����Ƿ�ƥ�䣬0-1�任
			s.seq=0xffffffff;
			to_physical_layer(&s);
		}
		//event=cksum_err ������ֱ�ӵȴ���һ���¼�
	}
}

void from_physical_layer(frame *s)
{
	char recvbuf[1036];
	read_share_file(recvbuf, "rpl_to_rdl", -1, 1036);
	memcpy(s, recvbuf, 1036);
	//ת��������
	s->ack=ntohl(s->ack);
	s->seq=ntohl(s->seq);
}

void to_physical_layer(frame *s)
{
	char buffer[1036];
	//ת��������
	s->ack=htonl(s->ack);
	s->seq=htonl(s->seq);
	memcpy(buffer, s, 1036);
	write_share_file(buffer, "rdl_to_rpl_pkg.dat", -1, 1036);
	kill(rpl_pid, SIG_RPL_SHOULD_READ);//������㷢�źţ�֪ͨ�䷢ACK֡
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
	
	if (signal(SIG_RPL_SHOULD_READ, sigroutine) < 0)//װ���ź�
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
	if(val=fcntl(conn,F_GETFL,0)<0){//��ȡ�ļ�״̬��־
		perror("fcntl");
		close(conn);
		exit(1);
	}
	if(fcntl(conn,F_SETFL,val|O_NONBLOCK)<0){//�����ļ�״̬��־
		perror("fcntl");
		close(conn);
		exit(1);
	}
	fd_set rfd,wfd;
	
	printf("here is ok!!!\n");
	while (1)
	{
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_SET(conn,&rfd);
		FD_SET(conn,&wfd);
		if(select(conn+1,&rfd,&wfd,NULL,0)>0){
			if(FD_ISSET(conn,&wfd)){//��д
				FD_CLR(conn,&wfd);
				if(rpl_send_flag==1){
					char buffer[12];
					read_share_file(buffer, "rdl_to_rpl_pkg.dat", -1, sizeof(buffer));
					write(conn, buffer, sizeof(buffer));
					printf("send a ack frame\n");
					rpl_send_flag=0;
				}
				else{
					usleep(1000*50);
				}
			}
			if(FD_ISSET(conn,&rfd)){//�ɶ�
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
					write_share_file(recvbuf, "rpl_to_rdl", -1, 1036); //�յ����ݺ�д�����ļ�
					kill(rdl_pid, SIG_RDL_SHOULD_READ);//����·�㷢�źţ�֪ͨ��������
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
	
	piddt.rpl_pidd=rpl_pid;//ͨ�������ļ���ʽ���ӽ���֪ͨ�����ӽ��̵�pid��ʹ֮֮����ʹ���ź�ͨ��
	piddt.rdl_pidd=rdl_pid;
	piddt.rnl_pidd=rnl_pid;
	
	write_share_file2((void*)&piddt,"pid_no",-1,sizeof(struct pidd));
	
	

	return 0;
}