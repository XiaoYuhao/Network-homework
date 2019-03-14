#include<stdio.h>
#include<unistd.h>
#include<strings.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#define FIFO_WR "read_fifo"
#define FIFO_RD "write_fifo"
#define MAXLEN 100
int main(){
	pid_t cpid;
    cpid=fork();
    if(cpid==-1||cpid>0)
        return 0;  
	
	int wfd,rfd;
	char sendbuf[MAXLEN]="test2-5-2 send to test2-5-1\n";
	char recvbuf[MAXLEN];
	rfd=open(FIFO_RD,O_RDONLY|O_NONBLOCK);
	if(rfd==-1){
		perror("open read");
		exit(1);
	}
	wfd=open(FIFO_WR,O_WRONLY);
	if(wfd==-1){
		perror("open write");
		exit(1);
	}
	while(1){
		sleep(1);
		memset(recvbuf,0,sizeof(recvbuf));
		read(rfd,recvbuf,sizeof(recvbuf));
		write(wfd,sendbuf,sizeof(sendbuf));
		printf("%s",recvbuf);
	}
	close(wfd);
	close(rfd);
	return 0;
}