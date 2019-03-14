#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>

#define FIFO_NAME "fifo"
#define MAXLEN 100

int main(){
	pid_t cpid;
    cpid=fork();
    if(cpid==-1||cpid>0)
        return 0;  
	
	int pid;
	
	unlink(FIFO_NAME);
	int res=mkfifo(FIFO_NAME,0666);
	if(res==-1){
		perror("mkfifo WRITE");
		exit(1);
	}
	
	pid=fork();
	if(pid==-1){
		printf("fork error!\n");
		exit(1);
	}
	
	
	if(pid==0){//子进程
		int wfd=open(FIFO_NAME,O_WRONLY);
		if(wfd==-1){
			perror("open WRITE");
			exit(1);
		}
		char sendbuf[MAXLEN]="child send to father\n";
		write(wfd,sendbuf,sizeof(sendbuf));
		sleep(1);
		int rfd=open(FIFO_NAME,O_RDONLY);
		if(rfd==-1){
			perror("open READ");
			exit(1);
		}
		char recvbuf[MAXLEN];
		if(read(rfd,recvbuf,sizeof(recvbuf))>0){
			printf("%s",recvbuf);
			memset(recvbuf,0,sizeof(recvbuf));
		}
		close(wfd);
		close(rfd);
	}
	else{//父进程
		int rfd=open(FIFO_NAME,O_RDONLY);
		if(rfd==-1){
			perror("open READ");
			exit(1);
		}
		char recvbuf[MAXLEN];
		if(read(rfd,recvbuf,sizeof(recvbuf))>0){
			printf("%s",recvbuf);
			memset(recvbuf,0,sizeof(recvbuf));
		}
		sleep(1);
		int wfd=open(FIFO_NAME,O_WRONLY);
		if(wfd==-1){
			perror("open WRITE");
			exit(1);
		}
		char sendbuf[MAXLEN]="father send to child\n";
		write(wfd,sendbuf,sizeof(sendbuf));
		close(wfd);
		close(rfd);
	}
}
