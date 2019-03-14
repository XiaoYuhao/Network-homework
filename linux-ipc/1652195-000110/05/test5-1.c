#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/shm.h>

#define MAXLEN 100

struct shared_use_st{
	int written1;
	int written2;
	char data1[MAXLEN];
	char data2[MAXLEN];
	int flag;
};

int main(){
	pid_t cpid;
    cpid=fork();
    if(cpid==-1||cpid>0)
        return 0; 
	
	void *shared_memory =(void*)0;
	struct shared_use_st *shared_stuff;
	int shmid;
	key_t key;
	key=ftok(".",1);

	shmid=shmget(key,sizeof(struct shared_use_st),0666|IPC_CREAT);
	if(shmid==-1){
		perror("shmget");
		exit(-1);
	}
	
	shared_memory = shmat(shmid,(void *)0,0);
	if(shared_memory==(void*)-1){
		perror("shmat");
		exit(-1);
	}
	
	shared_stuff=(struct shared_use_st *)shared_memory;
	shared_stuff->written1=0;
	shared_stuff->written2=0;
	shared_stuff->flag=0;
	
	int count=0;
	while(1){
		if(shared_stuff->written1){
			printf("%s",shared_stuff->data1);
			shared_stuff->written1=0;
			count++;
		}
		if(shared_stuff->written2==0){
			char sendbuf[MAXLEN]="test5-1 send to test5-2\n";
			strcpy(shared_stuff->data2,sendbuf);
			shared_stuff->written2=1;
		}
		if(count==5){
			shared_stuff->flag=1;
			break;
		}
		sleep(1);
	}
	
	
	if(shmdt(shared_memory)==-1){
		perror("shmdt");
		exit(1);
	}
	
	if(shmctl(shmid,IPC_RMID,0)==-1){
		perror("shmctl");
		exit(1);
	}
	
	return 0;
}