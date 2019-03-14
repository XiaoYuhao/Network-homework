//test3-2.c
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<stdlib.h>
int main(){
	pid_t pid;
	int i,j,k;
	
	pid=fork();//创建子进程
	
	if(pid<0)
	{
		printf("fork error\n");
		exit(1);
	}
	else if(pid>0){
		exit(0);//退出父进程
	}
	//下面使子进程成为守护进程
	setsid();//创建新会话
	chdir("/");//改变当前工作目录为根目录
	umask(0);//设置文件权限掩码
	
	
	pid=fork();
	
	if(pid>0){
		int status=0,ret;
		while(1){
			sleep(1);
			ret=waitpid(pid,&status,WNOHANG);
			if(ret){
				printf("%d号进程收到：%d号信号\n",pid,status);
				break;
			}
		}
	}
	
	else{
		while(1){
			sleep(1);
			exit(1);
		}
	}

	
	return 0;
}