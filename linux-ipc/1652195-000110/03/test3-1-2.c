#include<stdio.h>
#include<signal.h>
#include<stdlib.h>

#define MY_SIG1 50
#define MY_SIG2 51
#define MY_SIG3 52

void sig_fun1(int sig){
	printf("I got signal %d\n",sig);
}

void sig_fun2(int sig){
	printf("I got signal %d\n",sig);
}

void sig_fun3(int sig){
	printf("I got signal %d\n",sig);
	exit(1);
}

int main(){
	pid_t cpid;
    cpid=fork();
    if(cpid==-1||cpid>0)
        return 0;  
	
	if(signal(MY_SIG1,sig_fun1)==SIG_ERR){
		perror("signal sig_fun1");
		exit(1);
	}
	if(signal(MY_SIG2,sig_fun2)==SIG_ERR){
		perror("signal sig_fun2");
		exit(1);
	}
	if(signal(MY_SIG3,sig_fun3)==SIG_ERR){
		perror("signal sig_fun3");
		exit(1);
	}
	
	while(1){
		sleep(1);
	}
	return 0;
}