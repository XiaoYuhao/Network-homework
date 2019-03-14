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
	msgbuf.mtype=1;
	
	key=ftok(".",'a');
	
	msgid=msgget(key,IPC_CREAT|IPC_EXCL|0666);
	
	printf("msgid:%d\n",msgid);
	
	if(msgid==-1){
		if(errno==EEXIST){
			printf("EEXIST:......\n");
			key=ftok(".",'a');
			msgid=msgget(key,IPC_CREAT|0666);
		}
		else{
			perror("msgget");
			exit(-1);
		}
	}
	int i;
	for(i=1;i<=5;i++){
		char sendbuf[MAXLEN]="test4-1-1 send to test4-1-2\n";
		strcpy(msgbuf.data,sendbuf);
		
		if(msgsnd(msgid,(void *)&msgbuf,MAXLEN,0)==-1){
			perror("msgsnd");
			exit(-1);
		}
		printf("已发送%d条消息\n",i);
		sleep(1);
	}
	
	return 0;
}