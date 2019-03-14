#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>

#define MAXLEN 100

const char *test_file="/tmp/test_lock";

int main(){
	pid_t cpid;
    cpid=fork();
    if(cpid==-1||cpid>0)
        return 0; 
	
	int file_desc;
	struct flock region;
	int ret;
	
	file_desc=open(test_file,O_RDWR|O_CREAT,0666);
	if(!file_desc){
		perror("open file_desc");
		exit(1);
	}
	
	int val;
	if(val=fcntl(file_desc,F_GETFL,0)<0){//��ȡ�ļ�״̬��־
		perror("fcntl");
		close(file_desc);
		return 0;
	}
	if(fcntl(file_desc,F_SETFL,val|O_NONBLOCK)<0){//�����ļ�״̬��־
		perror("fcntl");
		close(file_desc);
		return 0;
	}
	
	printf("���ļ��ɹ�...\n");
	printf("׼�����ļ���д��...\n");
	region.l_type=F_WRLCK;
	region.l_whence=SEEK_SET;
	region.l_start=0;
	region.l_len=MAXLEN;
	
	ret=fcntl(file_desc,F_SETLKW,&region);
	if(ret==-1){
		perror("fcntl");
		exit(1);
	}
	printf("���ļ���д���ɹ�...\n");
	
	char sendbuf[MAXLEN]="test7-2-1 send to test7-1-1\n";
	ret=write(file_desc,sendbuf,sizeof(sendbuf));
	if(ret==-1){
		perror("write");
		exit(1);
	}
	
	region.l_type=F_UNLCK;
	region.l_whence=SEEK_SET;
	region.l_start=0;
	region.l_len=MAXLEN;
	
	ret=fcntl(file_desc,F_SETLKW,&region);
	if(ret==-1){
		perror("fcntl");
		exit(1);
	}
	printf("����ļ�д��...\n");
	
	while(1){
		sleep(1);
	}
	return 0;
}