#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<strings.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#define FIFO_WR "write_fifo"
#define FIFO_RD "read_fifo"
#define MAXLEN 100
int main(){
	pid_t cpid;
    cpid=fork();
    if(cpid==-1||cpid>0)
        return 0;  
	
	int wfd,rfd;
	unlink(FIFO_WR);
	unlink(FIFO_RD);
	char sendbuf[MAXLEN]="test2-5-1 send to test2-5-2\n";
	char recvbuf[MAXLEN];
	int ret=mkfifo(FIFO_WR,S_IFIFO|0666);
	if(ret==-1){
		perror("mkfifo write");
		exit(1);
	}
	ret=mkfifo(FIFO_RD,S_IFIFO|0666);
	if(ret==-1){
		perror("mkfifo read");
		exit(1);
	}
	wfd=open(FIFO_WR,O_WRONLY);
	if(wfd==-1){
		perror("open write");
		exit(1);
	}
	rfd=open(FIFO_RD,O_RDONLY|O_NONBLOCK);
	if(rfd==-1){
		perror("open read");
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
	unlink(FIFO_WR);	
	unlink(FIFO_RD);
	return 0;	
}