#include<stdio.h>
#include<unistd.h>
#include<strings.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<signal.h>

#define MY_SIG 50 // test3-2-1已经读完
#define MAXLEN 100

void sig_fun(){
	FILE *fp;
	int wfd,rfd;
	rfd=open("file.txt",O_RDONLY);
	char recvbuf[MAXLEN];
	int ret=read(rfd,recvbuf,sizeof(recvbuf));
	if(ret==-1){
		perror("read");
		exit(1);
	}
	else{
		printf("%s",recvbuf);
	}
	close(rfd);
	fp=fopen("file.txt","w");//清空文件
	fclose(fp);
	
	wfd=open("file.txt",O_WRONLY|O_CREAT);
	char sendbuf[MAXLEN]="test3-2-2 send to test3-2-1\n";
	ret=write(wfd,sendbuf,sizeof(sendbuf));
	if(ret==-1){
		perror("write");
		exit(1);
	}
	close(rfd);
	
	int pid=0;
	fp=popen("ps -e | grep \'test3-2-1\' | awk \'{print $1}\'","r");
	wait();//popen函数会通过fork产生子进程，若不处理此子进程，会产生僵尸进程
	char buf[10]={0};
	fgets(buf,10,fp);
	pid=atoi(buf);

	sleep(1);
	printf("pid:%d\n",pid);
	kill(pid,MY_SIG);
	return ;
}
int main(){
	pid_t cpid;
    cpid=fork();
    if(cpid==-1||cpid>0)
        return 0; 

	if(signal(MY_SIG,sig_fun)==SIG_ERR){//安装信号
		perror("signal sig_fun1");
		exit(1);
	}

	while(1){
		sleep(1);
	}

}