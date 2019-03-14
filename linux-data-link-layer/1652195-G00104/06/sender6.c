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


struct pidd {
	pid_t spl_pidd;
	pid_t sdl_pidd;
	pid_t snl_pidd;
};
#define inc(k) if (k < MAX_SEQ) k = k + 1; else k = 0;
#define MAX_SEQ 7
#define NR_BUFS ((MAX_SEQ+1)/2)
#define SIG_SPL_SHOULD_READ SIGRTMIN
#define SIG_NL_SHOULD_READ SIGRTMIN
#define SIG_FRAME_ARRIVAL 36
#define MAX_NETWORK_SHARE 1000
time_t timer[MAX_SEQ];
time_t ack_time;

#define MYTIMEOUT 5
#define SIG_SPL_CHSUM_ERR SIGRTMIN+1

bool no_nak = true;
seq_nr oldest_frame = MAX_SEQ + 1;

const char *send_source_file = "sender_test.txt";
const char *recv_data_file = "recver.test.txt";

pid_t pl_pid, dl_pid, nl_pid;

int network_write = 1;
int network_read = 0;
int network_go = 0;

int phy_read = 0;

int ready = 0;
int sockfd;

int err = 0;
int frame_ar = 0;

void signal_handle(int sig)
{
	if (sig == 38)
	{
		network_write = 1;
		network_go = 1;
	}
	if (sig == SIGRTMIN)                //34
	{
		network_read = 1;
		phy_read = 1;
	}
	if (sig == 39)
		network_write = 0;
	if (sig == 37)
		ready = 1;
	if (sig == 35)
		err = 1;
	if (sig == 36)
		frame_ar = 1;
}

void Network_Layer()          //没有加锁
{
	printf("Here is network layer!\n");
	struct pidd piddt;
	read_share_file2((void*)&piddt, "pid_no", -1, sizeof(struct pidd));
	pl_pid = piddt.spl_pidd;
	dl_pid = piddt.sdl_pidd;
	nl_pid = piddt.snl_pidd;
	struct shared_bool_memory *shared = create_share_bool_memory(1234);
	int share_file_pointer = 0;
	FILE *fp;
	if (network_write)
	{
		fp = fopen(send_source_file, "r");
		if (fp == NULL)
			perror_exit("fopen");
	}
	signal(38, signal_handle);
	signal(39, signal_handle);
	signal(34, signal_handle);
	int wfinish = 0;
	while (1)
	{
		if (network_write&&wfinish == 0)
		{
			packet buffer;
			size_t len;
			if (len = fread(buffer.data, 1, MAX_PKT, fp))   //不足怎么办
			{
				while (shared->written1 == 1) {
					usleep(1000 * 500);
					printf("shared->written=1");
				}
				printf("write:network_datalink.share.%d\n", share_file_pointer);
				write_share_file(buffer.data, "network_datalink.share.", share_file_pointer, MAX_PKT);
				shared->written2 = 1;
				shared->text[share_file_pointer] = 1;
				shared->written2 = 0;
				printf("%d\n", share_file_pointer);
				share_file_pointer = (++share_file_pointer) % MAX_NETWORK_SHARE;

				//发送信号
				kill(dl_pid, 37);
				while (network_go == 0)
					usleep(50 * 1000);
				network_go = 0;
			}
			else
			{
				printf("文件传输完毕!\n");
				wfinish = 1;
			}

		}
		if (network_read)
		{
			char recvbuf[1024];
			read_share_file(recvbuf, "rdl_to_rnl", -1, 1024);
			//printf("recvbuf:%s\n",recvbuf);
			FILE *fp = fopen(recv_data_file, "ab+");
			if (fp == NULL)
			{
				perror_exit("fopen");
			}
			fwrite(recvbuf, 1, 1024, fp);
			//printf("RNL has write %d packet\n", ++sum);
			network_read = 0;
			fclose(fp);
		}
		usleep(50 * 1000);
	}
}


void from_network_layer(packet *buffer)
{
	if (network_write == 0)
	{
		printf("错误调用!\n");
		exit(0);
	}
	struct shared_bool_memory* shared = get_share_bool_memory(1234);

	static int first_readable = 0;
	while (shared->text[first_readable] == 0) {
		usleep(1000 * 500);
		first_readable = (++first_readable) % MAX_NETWORK_SHARE;
		//	printf("???????\n");
	}
	while (shared->written2 == 1) {
		usleep(1000 * 500);
		printf("shared->written=1");
	}
	shared->written1 = 1;
	shared->text[first_readable] = 0;
	shared->written1 = 0;
	printf("read:network_datalink.share.%d\n", first_readable);
	
	read_share_file(buffer->data, "network_datalink.share.", first_readable, MAX_PKT);
	
	kill(nl_pid, 38);
}


void enable_network_layer()
{
	kill(nl_pid, 38);
}

void disable_network_layer()
{
	kill(nl_pid, 39);
}


void start_timer(seq_nr k)
{
	time_t t;
	timer[k % MAX_SEQ] = time(&t);
}

void stop_timer(seq_nr k)
{
	timer[k % MAX_SEQ] = 0;
}

void start_ack_timer()
{
	time_t t;
	ack_time= time(&t);
}

void stop_ack_timer()
{
	ack_time = 0;
}


void wait_for_event(event_type *event)
{
	if (err == 1)
	{
		*event = cksum_err;
		err = 0;
		return;
	}
	
	int i;
	time_t t;
	time(&t);
	for (i = 0; i < MAX_SEQ; i++)
		if (timer[i] != 0 && t - timer[i] > MYTIMEOUT)
		{
			*event = timeout;
			oldest_frame = i;
			return;
		}
	if (ack_time!=0&&t-ack_time > MYTIMEOUT)
	{
		*event = ack_timeout;
		return;
	}
	if (ready == 1)
	{
		*event = network_layer_ready;
		ready = 0;
		return;
	}

	if (frame_ar == 1)
	{
		*event = frame_arrival;
		frame_ar = 0;
		return;
	}
	

}

static bool between(seq_nr a, seq_nr b, seq_nr c)
{
	return (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)));
}


static void send_frame(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, packet buffer[])
{
	frame s;
	s.kind = fk;
	if (fk == data)
		strcpy(s.info.data,buffer[frame_nr%NR_BUFS].data);
	s.seq = frame_nr;
	s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);
	if (fk == nak)
		no_nak = false;
	to_physical_layer(&s);
	if (fk == data)
		start_timer(frame_nr%NR_BUFS);
	stop_ack_timer();
}



void to_physical_layer(frame *s)
{
	char buffer[1036];
	memcpy(buffer, s, 1036);
	write_share_file(buffer, "sdl_to_spl_pkg.dat", -1, 1036);
	kill(pl_pid, SIG_SPL_SHOULD_READ);
}


void from_physical_layer(frame *s)
{
	char recvbuf[1036];
	read_share_file(recvbuf, "spl_to_sdl", -1, 1036);
	memcpy(s, recvbuf, 1036);
}


void Physics_Layer(int server_port, const char *server_ip)             //send
{
	struct pidd piddt;
	read_share_file2((void*)&piddt, "pid_no", -1, sizeof(struct pidd));
	pl_pid = piddt.spl_pidd;
	dl_pid = piddt.sdl_pidd;
	nl_pid = piddt.snl_pidd;
	//printf("SPL spl_pid:%d sdl_pid:%d snl_pid:%d\n", spl_pid, sdl_pid, snl_pid);

	int socket = init_sender_socket(server_port, server_ip);

	if (signal(SIG_SPL_SHOULD_READ, signal_handle) < 0)
	{
		perror_exit("signal");
	}
	int sum = 0;

	int val;
	if (val = fcntl(socket, F_GETFL, 0) < 0) {//获取文件状态标志
		perror("fcntl");
		close(socket);
		exit(1);
	}
	if (fcntl(socket, F_SETFL, val | O_NONBLOCK) < 0) {//设置文件状态标志
		perror("fcntl");
		close(socket);
		exit(1);
	}

	signal(SIG_SPL_SHOULD_READ, signal_handle);
	fd_set rfd, wfd;

	while (1)
	{
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_SET(socket, &rfd);
		FD_SET(socket, &wfd);
		if (select(socket + 1, &rfd, &wfd, NULL, 0) > 0) {
			if (FD_ISSET(socket, &wfd)) {
				FD_CLR(socket, &wfd);
				if (phy_read == 1)
				{
					char buffer[1036];
					read_share_file(buffer, "sdl_to_spl_pkg.dat", -1, 1036);
					//sum++;
					//printf("send_num:%d\n", sum);
					write(socket, buffer, 1036);
					phy_read = 0;
				}
				else {
					usleep(1000 * 50);
					//printf("wait for data from SDL\n");
				}
			}
			if (FD_ISSET(socket, &rfd)) {
				FD_CLR(socket, &rfd);
				char buffer[1036];
				int ret = read(socket, buffer, sizeof(buffer));
				if (ret <= 0) {
					perror("read");
					exit(2);
				}
				int t = rand() % 100;
				if (t < 3)kill(dl_pid, SIG_SPL_CHSUM_ERR);
				else if (t > 96) {
					frame *fpointer = (frame *)buffer;
					if (fpointer->kind == ack)           //引发ack_timeout
						ack_time = 1;
				}
				else {
					write_share_file(buffer, "spl_to_sdl", -1, 1036);
					kill(dl_pid, SIG_FRAME_ARRIVAL);
				}

			}

		}
	}
	
}

void to_network_layer(packet *p)
{
	char sendbuf[1024];
	memcpy(sendbuf, p, 1024);
	//printf("sendbuf:%s\n",sendbuf);
	write_share_file(sendbuf, "rdl_to_rnl", -1, 1024); //写共享文件
	kill(nl_pid, 34);//向网络层发信号，读取数据
}

void Datalink_Layer()
{
	signal(SIG_SPL_CHSUM_ERR, signal_handle);
	signal(SIG_FRAME_ARRIVAL, signal_handle);
	signal(37,signal_handle);
	struct pidd piddt;
	read_share_file2((void*)&piddt, "pid_no", -1, sizeof(struct pidd));
	pl_pid = piddt.spl_pidd;
	dl_pid = piddt.sdl_pidd;
	nl_pid = piddt.snl_pidd;
	seq_nr ack_expected,next_frame_to_send;
	seq_nr frame_expected,too_far;
	int i;
	frame r;
	packet out_buf[NR_BUFS];
	packet in_buf[NR_BUFS];
	bool arrived[NR_BUFS];
	seq_nr nbuffered;
	event_type event;
	enable_network_layer();
	ack_expected = 0;
	next_frame_to_send = 0; //初始帧号为0
	frame_expected = 0;
	too_far = NR_BUFS; //初始为非法(合法0..NF_BUFS-1)
	nbuffered = 0;
	for (i = 0; i < NR_BUFS; i++)
		arrived[i] = false;
	while (true)
	{
		wait_for_event(&event);
		switch (event) 
		{
		case network_layer_ready:
			nbuffered = nbuffered + 1;
			from_network_layer(&out_buf[next_frame_to_send % NR_BUFS]);
			send_frame(data, next_frame_to_send, frame_expected, out_buf);
			inc(next_frame_to_send);
			break;
		case frame_arrival:
			from_physical_layer(&r);
			if (r.kind == data)
			{
				if ((r.seq != frame_expected) && no_nak)
					send_frame(nak, 0, frame_expected, out_buf);
				else
					start_ack_timer();
				if (between(frame_expected, r.seq, too_far) && arrived[r.seq%NR_BUFS] == false)
				{
					arrived[r.seq%NR_BUFS] = true;
					in_buf[r.seq%NR_BUFS] = r.info; //放入接收窗中
					while (arrived[frame_expected % NR_BUFS])
					{
						to_network_layer(&in_buf[arrived[frame_expected % NR_BUFS]]);
						no_nak = true;//全局量，置true表示不发nak
						arrived[frame_expected % NR_BUFS] = false; //清接收窗口
						inc(frame_expected);
						inc(too_far); //如果初始，则为0，以后正常
						start_ack_timer();
					}

				} // end of if(r.kind==data)
				/* 如果发送nak，则找出最后一个确认帧序号的下一个 */
				if ((r.kind == nak) && between(ack_expected, (r.ack + 1) % (MAX_SEQ + 1), next_frame_to_send))
					send_frame(data, (r.ack + 1) % (MAX_SEQ + 1), frame_expected, out_buf);
				/* 处理收到的ack（独立帧或数据帧捎带过来）*/
				while (between(ack_expected, r.ack, next_frame_to_send))
				{
					nbuffered = nbuffered - 1;
					stop_timer(ack_expected % NR_BUFS);//链表中删除
					inc(ack_expected);
				}
			}
				break;
		case cksum_err:
			if (no_nak) //没发过nak则发nak
				send_frame(nak, 0, frame_expected, out_buf);
			break;
		case timeout: //数据包超时则重发数据包
			send_frame(data, oldest_frame, frame_expected, out_buf);
			break;
		case ack_timeout: //ack超时则单独发ack包(未被捎带的情况)
			send_frame(ack, 0, frame_expected, out_buf);
			break;
		} //end of switch
		if (nbuffered < NR_BUFS)
			enable_network_layer(); //允许上层发数据
		else
			disable_network_layer(); //不允许上层发数据
	} // end of while
}







int main(int argc, const char *argv[])
{
	int server_port = atoi(argv[1]);
	const char *server_ip = argv[2];

	delete_share_bool_memory(1234);

	nl_pid = fork();
	if (nl_pid < 0)
	{
		perror_exit("fork");
		exit(EXIT_FAILURE);
	}
	else if (nl_pid == 0)
	{
		Network_Layer();
		printf("NL exit\n");
		exit(EXIT_SUCCESS);
	}

	pl_pid = fork();

	if (pl_pid < 0)
	{
		perror_exit("fork");
		exit(EXIT_FAILURE);
	}
	else if (pl_pid == 0)
	{
		Physics_Layer(server_port, server_ip);
		printf("PL exit\n");
		exit(EXIT_SUCCESS);
	}

	dl_pid = fork();
	if (dl_pid < 0)
	{
		perror_exit("fork");
		exit(EXIT_FAILURE);
	}
	else if (dl_pid == 0)
	{
		Datalink_Layer();
		printf("DL exit\n");
		exit(EXIT_SUCCESS);
	}

	struct pidd piddt;

	piddt.spl_pidd = pl_pid;//通过共享文件方式向子进程通知各个子进程的pid，使之之后能使用信号通信
	piddt.sdl_pidd = dl_pid;
	piddt.snl_pidd = nl_pid;

	write_share_file2((void*)&piddt, "pid_no", -1, sizeof(struct pidd));

	int status;
	while (1) {
		pid_t p = waitpid(-1, &status, 0);
		printf("exit:%d\n", status);
	}
	return 0;
}