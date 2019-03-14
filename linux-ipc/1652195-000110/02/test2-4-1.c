#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<strings.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#define FIFO_NAME "fifo"
#define MAXLEN 100
int main(){
	pid_t cpid;
    cpid=fork();
    if(cpid==-1||cpid>0)
        return 0;  
	
	int fd;
	unlink(FIFO_NAME);
	char sendbuf[MAXLEN]="test2-4-1 send to test2-4-2\n";
	int ret=mkfifo(FIFO_NAME,S_IFIFO|0666);
	if(ret==-1){
		perror("mkfifo");
		exit(1);
	}
	fd=open(FIFO_NAME,O_WRONLY);
	if(fd==-1){
		perror("open");
	}
	write(fd,sendbuf,sizeof(sendbuf));
	close(fd);
	unlink(FIFO_NAME);	
	sleep(1);
	return 0;	
}