#include<stdio.h>
#include<unistd.h>
#include<strings.h>
#include<string.h>
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
	char recvbuf[MAXLEN];
	fd=open(FIFO_NAME,O_RDONLY);
	if(fd==-1){
		perror("open");
	}
	read(fd,recvbuf,sizeof(recvbuf));
	printf("%s\n",recvbuf);
	close(fd);
	return 0;
}