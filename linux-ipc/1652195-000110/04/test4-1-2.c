#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/msg.h>
#include<sys/ipc.h>
#include<unistd.h>
#include<errno.h>

#define MAXLEN 512
struct msg_buf
{
	long mtype;
	char data[MAXLEN];
};

int main(){
	pid_t cpid;
    cpid=fork();
    if(cpid==-1||cpid>0)
        return 0; 
	
	key_t key;
	int msgid;
	int ret;
	struct msg_buf msgbuf;
	long msg_to_receive=0;
	
	key=ftok(".",'a');
	
	msgid=msgget(key,IPC_CREAT|0666);
	
	printf("msgid:%d\n",msgid);
	
	if(msgid==-1){
		perror("msgget");
		exit(1);
	}
	int count=0;
	for(count=1;count<=5;count++){
		printf("正在接受第%d条消息\n",count);
		if(msgrcv(msgid,(void *)&msgbuf,MAXLEN,msg_to_receive,0)==-1){
			perror("msgrcv");
			exit(-1);
		}
		printf("%s",msgbuf.data);
	}
	if(msgctl(msgid,IPC_RMID,0)==-1){
		perror("msgctl");
		exit(-1);
	}
	
	return 0;
}