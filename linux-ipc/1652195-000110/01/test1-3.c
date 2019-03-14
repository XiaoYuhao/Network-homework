#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<stdlib.h>
#include<wait.h>
#include<string.h>

#define LENGTH 50

int main(){
	pid_t cpid;
    cpid=fork();
    if(cpid==-1||cpid>0)
        return 0;  
	
	int status;
	int pipefd[2];
	int pipefds[2];
	int ret;
	pid_t pid;
	
	ret=pipe(pipefd);
	if(ret==-1)
	{
		perror("pipe");
		exit(1);
	}
	ret=pipe(pipefds);
	if(ret==-1)
	{
		perror("pipe");
		exit(1);
	}
	
	pid=fork();
	
	if(pid==-1)
	{
		printf("fork error!\n");
		exit(1);
	}
	
	char buf[LENGTH]={0};
	
	if(pid==0){
		close(pipefd[1]);
		close(pipefds[0]);
		strcpy(buf,"child send to father\n");
		write(pipefds[1],buf,strlen(buf));
		close(pipefds[1]);
		while(read(pipefd[0],buf,sizeof(buf))>0){
			printf("%s",buf);
			memset(buf,0,sizeof(buf));
		}
		close(pipefd[0]);
	}
	else{
		close(pipefds[1]);
		close(pipefd[0]);
		strcpy(buf,"father send to child\n");
		write(pipefd[1],buf,strlen(buf));
		close(pipefd[1]);
		while(read(pipefds[0],buf,sizeof(buf))>0){
			printf("%s",buf);
			memset(buf,0,sizeof(buf));
		}
		close(pipefds[0]);
		wait(&status);
	}
	return 0;
}