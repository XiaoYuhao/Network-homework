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

#define SIG_SPL_SHOULD_READ SIGRTMIN
#define MAX_NETWORK_SHARE 1000
const char *source_file = "sender_test.txt";
pid_t spl_pid, sdl_pid, snl_pid;

void SNL()
{
	printf("SNL is running\n");
	int share_file_pointer = 0;

	FILE *fp = fopen(source_file, "r");
	if (fp == NULL)
		perror_exit("fopen");

	packet buffer;
	struct shared_bool_memory *shared = create_share_bool_memory(1234);
	size_t len;
	while (len = fread(buffer.data, 1, MAX_PKT, fp))
	{
		while(shared->written1==1){
			usleep(1000*500);
			printf("shared->written=1");
		}
		//printf("write:network_datalink.share.%d\n",share_file_pointer);
		write_share_file(buffer.data, "network_datalink.share.", share_file_pointer, len);
		shared->written2=1;
		shared->text[share_file_pointer] = 1;
		shared->written2=0;
		printf("%d\n",share_file_pointer);
		share_file_pointer = (++share_file_pointer) % MAX_NETWORK_SHARE;
		memset(buffer.data,'\0',1024);
	}
	printf("file has been read\n");
	for(int i=0;i<1024;i++){
		buffer.data[i]='\0';
	}
	write_share_file(buffer.data, "network_datalink.share.", share_file_pointer, len);
	shared->written2=1;
	shared->text[share_file_pointer] = 1;
	shared->written2=0;
	share_file_pointer = (++share_file_pointer) % MAX_NETWORK_SHARE;
	int i=0;
	for(i=0;i<30;i++){
		printf("%d ",shared->text[i]);
	}
	printf("\n");
	fclose(fp);
}

void from_network_layer(packet *buffer)
{
	struct shared_bool_memory* shared = get_share_bool_memory(1234);

	static int first_readable = 0;
	while (shared->text[first_readable] == 0){
		usleep(1000*500);
		first_readable = (++first_readable) % MAX_NETWORK_SHARE;
	}
	while(shared->written2==1){
		usleep(1000*500);
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
	memcpy(buffer, s, 1036);
	write_share_file(buffer, "sdl_to_spl_pkg.dat", -1, 1036);
	kill(spl_pid, SIG_SPL_SHOULD_READ);
}

void SDL(pid_t SPL_pid)
{
	printf("SDL is running\n");
	spl_pid = SPL_pid;
	frame s;	   /* buffer for an outbound frame */
	packet buffer; /* buffer for an outbound packet */

	while (true)
	{
		from_network_layer(&buffer); /* go get something to send */
		//printf("from_network_layer is ok!\n");
		memcpy(&s.info,&buffer.data,1024);
		to_physical_layer(&s);		 /* send it on its way */
	}
}

bool spl_should_read = false;

void sigroutine(int sig)
{
	if (sig == SIG_SPL_SHOULD_READ)
	{
		spl_should_read = true;
		//printf("spl_should_read sig has been recv\n");
	}
	else
	{
		printf("Unexpected sig %d\n", sig);
		exit(EXIT_FAILURE);
	}
}

void SPL(int server_port, const char *server_ip)
{
	printf("SPL is running\n");
	int socket = init_sender_socket(server_port, server_ip);

	if (signal(SIG_SPL_SHOULD_READ, sigroutine) < 0)
	{
		perror_exit("signal");
	}
	int sum=0;
	while (true)
	{
		if (spl_should_read == true)
		{
			char buffer[1036];
			read_share_file(buffer, "sdl_to_spl_pkg.dat", -1, 1036);

			sum++;
			printf("send_num:%d\n",sum);
			write(socket, buffer, 1036);
			spl_should_read = false;
		}
		else{
			usleep(1000*1000);
			//printf("wait for data from SDL\n");
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
		sleep(1);
		SDL(spl_pid);
		printf("SDL is exit\n");
		exit(EXIT_SUCCESS);
	}

	
	int status;
	while(1){
		pid_t p=waitpid(-1,&status,0);
		printf("exit:%d\n",status);
	}
	
	return 0;
}