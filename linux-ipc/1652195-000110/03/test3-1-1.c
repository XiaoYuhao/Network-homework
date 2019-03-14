#include<stdio.h>
#include<signal.h>
#include<string.h>
#include<stdlib.h>

#define MY_SIG1 50
#define MY_SIG2 51
#define MY_SIG3 52

int main(){
	pid_t cpid;
    cpid=fork();
    if(cpid==-1||cpid>0)
        return 0; 

	FILE *fp = popen("ps -e | grep \'test3-1-2\' | awk \'{print $1}\'","r");
	char buf[10]={0};
	fgets(buf,10,fp);
	int pid=atoi(buf);
	kill(pid,MY_SIG1);
	sleep(1);
	kill(pid,MY_SIG2);
	sleep(1);
	kill(pid,MY_SIG3);
	sleep(1);

}